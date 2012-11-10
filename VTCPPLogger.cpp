#include "VTCPPLogger.h"

// sink output into void
std::shared_ptr<VT::detail_::onullstream> VT::Logger::dev_null_ = std::shared_ptr<VT::detail_::onullstream>(new VT::detail_::onullstream());