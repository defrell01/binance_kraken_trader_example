#include <bot/logger/logger.hpp>

void initLogger()
{
     auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
     consoleSink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
 
     auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("bot.log", true);
     fileSink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
 
     spdlog::sinks_init_list sinks{ consoleSink, fileSink };
 
     auto logger = std::make_shared<spdlog::logger>("bot_logger", sinks.begin(), sinks.end());
     logger->set_level(spdlog::level::info);
     spdlog::set_default_logger(logger);
     spdlog::flush_on(spdlog::level::info);
}