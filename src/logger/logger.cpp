#include <veriblock/logger/logger.hpp>

static std::shared_ptr<Logger> logger_;

void SetLogger(std::shared_ptr<Logger> logger) { logger_ = std::move(logger); }

std::shared_ptr<Logger> GetLogger() { return logger_; }
