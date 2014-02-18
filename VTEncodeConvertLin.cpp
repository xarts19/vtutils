#include "VTEncodeConvert.hpp"

#include "VTUtil.h"

#include <string>
#include <memory>
#include <stdexcept>
#include <wchar.h>
#include <assert.h>
#include <string.h>

#include <iconv.h>
#include <unistd.h>


//
// Determines iconv() behavior when encountering an invalid multibyte sequence:
//   "//TRANSLIT" - approximate through one or several characters that look similar to the original character
//   "//IGNORE"   - silently discard invalid character
//   ""           - iconv returns EILSEQ
//
#define OUT_CHARSET_PARAMETERS "//TRANSLIT"

#define CHARSET_WCHAR_T       "WCHAR_T" OUT_CHARSET_PARAMETERS
#define CHARSET_UTF8          "UTF-8" OUT_CHARSET_PARAMETERS

class iconv_wrap
{
public:
    // modifier_hint - hint for how much to scale output buffer size
    iconv_wrap(const char* from_charset, const char* to_charset, int modifier_hint = 1)
        : modifier_(modifier_hint)
        , descriptor_(iconv_open(to_charset, from_charset))
    {
        if (descriptor_ == (iconv_t)-1)
        {
            if (errno == EINVAL)
                throw std::runtime_error(std::string("Conversion from ") + from_charset + " to " +
                                         to_charset + " is not supported by the implementation.");
            else
                throw std::runtime_error("Failed to initialize character conversion");
        }

        if (modifier_ < 1)
            modifier_ = 1;
        else if (modifier_ > 4)
            modifier_ = 4;
    }

    ~iconv_wrap()
    {
        int res = iconv_close(descriptor_);
        assert(res == 0);
        VT_UNUSED(res);
    }

    template <typename DestT, typename SrcT>
    std::basic_string<DestT> convert(const std::basic_string<SrcT>& input)
    {
        return convert<DestT>(input.c_str(), input.size());
    }

    template <typename DestT, typename SrcT>
    std::basic_string<DestT> convert(const SrcT* input, size_t input_size = 0)
    {
        if (input_size == 0)
            input_size = measure(input);

        size_t res_size = 0;
        auto res = convert_helper(reinterpret_cast<const char*>(input), input_size * sizeof(SrcT), &res_size);

        assert(res_size % sizeof(DestT) == 0);
        size_t char_size = res_size / sizeof(DestT);
        return std::basic_string<DestT>(reinterpret_cast<DestT*>(res.get()), char_size);
    }

private:
    // Find length of the string
    template<typename Ch>
    inline size_t measure(const Ch* p)
    {
        const Ch* tmp = p;
        while (*tmp)
            ++tmp;
        return tmp - p;
    }

    /*
     * input - input string
     * input_size - size of input string in characters, without nul-character
     * output_size - size of the output string in characters, without nul-character
     */
    std::unique_ptr<char, void(*)(void*)> convert_helper(const char* input, size_t input_size, size_t* output_size)
    {

        size_t inleft = input_size;
        size_t outleft = 0;
        size_t converted = 0;

        // Output buffer is the size of input buffer scaled by our modifier.
        // We don't scale the buffer down, because it will be used to create string of
        // the exact needed side and freed, so no storage will be wasted.
        size_t outlen = input_size * modifier_;

        const char* inbuf = input;
        char* output = nullptr;
        char* outbuf = nullptr;

        // We allocate 4 bytes more than what we need for nul-termination...
        // Also, malloc guarantees alignment for any type if we decide to cast in to something later.
        output = (char*)malloc(outlen + 4);

        if (!output)
            throw std::bad_alloc();

        for (;;)
        {
            errno = 0;
            outbuf = output + converted;
            outleft = outlen - converted;

            converted = iconv(descriptor_, (char**)&inbuf, &inleft, &outbuf, &outleft);

            // Success
            if (converted != (size_t)-1)
                break;

            // EINVAL An incomplete  multibyte sequence has been encoun­tered in the input.
            // We'll just truncate it and ignore it.
            if (errno == EINVAL)
                break;

            // EILSEQ An invalid multibyte sequence has been  encountered in the input.
            // Bad input, we can't really recover from this.
            if (errno == EILSEQ)
            {
                free(output);
                throw std::runtime_error("Bad multibyte sequence");
            }

            // E2BIG There is not sufficient room at *outbuf.
            // We just need to grow our outbuffer and try again.
            if (errno == E2BIG)
            {
                converted = outbuf - output;

                // more_left == 4 - 3/4 left
                // more_left == 2 - half left
                // more_left == 1 - nothing left
                size_t more_left = (input_size + 1) / (input_size - inleft + 1);
                int shift;

                if (more_left >= 8)
                    shift = 3;  // multiply by 8
                else if (more_left >= 4)
                    shift = 2;  // multiply by 4
                else
                    shift = 1;  // multiply by 2

                outlen += (inleft << shift) + 8;


                // make output size divisible by 4 (assuming 4 is the maximum size of DestT character)
                outlen -= outlen % 4;

                char* tmp = (char*)realloc(output, outlen + 4);
                if (!tmp)
                {
                    free(output);
                    throw std::bad_alloc();
                }

                output = tmp;
                outbuf = output + converted;
                continue;
            }

            assert(false && "unreachable");
        }

        // flush the iconv conversion
        iconv(descriptor_, nullptr, nullptr, &outbuf, &outleft);

        /* Note: not all charsets can be nul-terminated with a single
         * nul byte. UCS2, for example, needs 2 nul bytes and UCS4
         * needs 4. I hope that 4 nul bytes is enough to terminate all
         * multibyte charsets */

        /* nul-terminate the string */
        memset(outbuf, 0, 4);

        *output_size = outbuf - output;
        return std::unique_ptr<char, void(*)(void*)>(output, free);
    }

    int modifier_;
    iconv_t descriptor_;
};


template <>
inline size_t iconv_wrap::measure<char>(const char* p)
{
    return strlen(p);
}


template <>
inline size_t iconv_wrap::measure<wchar_t>(const wchar_t* p)
{
    return wcslen(p);
}


namespace VT
{

    std::wstring UTF8ToWstring(const std::string& input)
    {
        return iconv_wrap(CHARSET_UTF8, CHARSET_WCHAR_T, 4).convert<wchar_t>(input);
    }

    std::wstring UTF8ToWstring(const char* input)
    {
        return iconv_wrap(CHARSET_UTF8, CHARSET_WCHAR_T, 4).convert<wchar_t>(input);
    }

    std::string WstringToUTF8(const std::wstring& input)
    {
        return iconv_wrap(CHARSET_WCHAR_T, CHARSET_UTF8).convert<char>(input);
    }

    std::string WstringToUTF8(const wchar_t* input)
    {
        return iconv_wrap(CHARSET_WCHAR_T, CHARSET_UTF8).convert<char>(input);
    }

    std::wstring AnsiToWstring(const std::string& /*input*/, const char* /*locale*/ /*= ".ACP" */)
    {
        throw std::runtime_error("VT::AnsiToWstring is not implemented");
    }

    std::string WstringToAnsi( const std::wstring& /*input*/, const char* /*locale*/ /*= ".ACP" */ )
    {
        throw std::runtime_error("VT::WstringToAnsi is not implemented");
    }
}
