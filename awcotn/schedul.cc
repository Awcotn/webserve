 #include "schedul.h"
 #include "log.h"

namespace awcotn {

static awcotn::Logger::ptr g_logger = AWCO_LOG_NAME("system");

Scheduler::Scheduler(size_t threads = 1, bool use_caller = true, const std::string& name = "") {

}

Scheduler::~Scheduler() {

}

Scheduler* Scheduler::GetThis() {

}

Fiber* Scheduler::GetMainFiber() {

}

void Scheduler::start() {

}

void Scheduler::stop() {

}

}