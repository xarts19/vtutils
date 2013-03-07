/*
* Copyright (c) 2008, Power of Two Games LLC
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of Power of Two Games LLC nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY POWER OF TWO GAMES LLC ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL POWER OF TWO GAMES LLC BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#pragma once

#ifndef NDEBUG
    #define VT_ASSERTS_ENABLED
#endif

namespace Assert
{
	enum FailBehavior
	{
		Halt,
		Continue
	};

	typedef FailBehavior (*Handler)(const char* condition, 
									const char* msg, 
									const char* file, 
									int line);

	Handler GetHandler();
	void SetHandler(Handler newHandler);

	FailBehavior ReportFailure(const char* condition, 
							   const char* file, 
							   int line, 
							   const char* msg, ...);
}

#ifdef _MSC_VER
#define VT_HALT() __debugbreak()
#else
#include <signal.h>
#define VT_HALT() raise(SIGTRAP)
#endif

#ifdef VT_ASSERTS_ENABLED
	#define VT_ASSERT(cond) \
		do \
		{ \
			if (!(cond)) \
			{ \
				if (Assert::ReportFailure(#cond, __FILE__, __LINE__, 0) == \
					Assert::Halt) \
					VT_HALT(); \
			} \
		} while(0)

	#define VT_ASSERT_MSG(cond, msg, ...) \
		do \
		{ \
			if (!(cond)) \
			{ \
				if (Assert::ReportFailure(#cond, __FILE__, __LINE__, (msg), __VA_ARGS__) == \
					Assert::Halt) \
					VT_HALT(); \
			} \
		} while(0)

	#define VT_ASSERT_FAIL(msg, ...) \
		do \
		{ \
			if (Assert::ReportFailure(0, __FILE__, __LINE__, (msg), __VA_ARGS__) == \
				Assert::Halt) \
			VT_HALT(); \
		} while(0)

	#define VT_VERIFY(cond) VT_ASSERT(cond)
	#define VT_VERIFY_MSG(cond, msg, ...) VT_ASSERT_MSG(cond, msg, ##__VA_ARGS__)
#else
	#define VT_ASSERT(condition) \
		do { VT_UNUSED(condition); } while(0)
	#define VT_ASSERT_MSG(condition, msg, ...) \
		do { VT_UNUSED(condition); VT_UNUSED(msg); } while(0)
	#define VT_ASSERT_FAIL(msg, ...) \
		do { VT_UNUSED(msg); } while(0)
	#define VT_VERIFY(cond) (void)(cond)
	#define VT_VERIFY_MSG(cond, msg, ...) \
		do { (void)(cond); VT_UNUSED(msg); } while(0)
#endif

#define VT_STATIC_ASSERT(x) \
	typedef char pow2StaticAssert[(x) ? 1 : -1];
