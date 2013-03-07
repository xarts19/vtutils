#include "VTAssert.h"
#include "VTUtil.h"
#include "VTStringUtil.h"
#include "VTTimer.h"

int main()
{
    VT_ASSERT(true);
    
    VT::Timer timer;
    
    std::vector<int> cont;
    VT::Utils::insert_if_not_present(cont, 4);
    VT_ASSERT(cont.size() == 1 && cont[0] == 4);
    
    std::vector<std::string> contspl;
    VT::StrUtils::split("one,two,three", &contspl, ",");
    VT_ASSERT(contspl.size() == 3 && contspl[0] == "one" && contspl[1] == "two" && contspl[2] == "three");
    
    VT_ASSERT(VT::StrUtils::trim("  Bus     ") == "Bus");
    VT_ASSERT(VT::StrUtils::trim("Bus") == "Bus");
    VT_ASSERT(VT::StrUtils::trim("a Bus") == "a Bus");
    VT_ASSERT(VT::StrUtils::starts_with("good day", "good") == true);
    VT_ASSERT(VT::StrUtils::starts_with("good day", "bad") == false);
    VT_ASSERT(VT::StrUtils::starts_with(" good day", "good") == false);
    
    VT_ASSERT(VT::StrUtils::to_lower("BaUojdUIOnsa adf DDF") == "bauojduionsa adf ddf");

    printf("Test ran in %f s\n", timer.time_elapsed_s());
    
    return 0;
}

