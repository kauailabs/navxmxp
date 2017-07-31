/*
 * Logger.h
 *
 *  Created on: 28 Jul 2017
 *      Author: pi
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include <fstream>
#include <iostream>
#include <string>

enum LogLevel {
    ERROR_LEVEL, WARN_LEVEL, INFO_LEVEL, DEBUG_LEVEL, TRACE_LEVEL
};

class LoggerBackend {
public:
    virtual void output_line(LogLevel level, std::string string) = 0;
    virtual ~LoggerBackend() {}
};

class FileLogger : public LoggerBackend {
    std::ofstream out_stream;
public:
    FileLogger()
    {
    }

    FileLogger(const char * filename)
    {
        open(filename);
    }

    bool open(const char * filename)
    {
        if (!out_stream.is_open()) {
            out_stream.open(filename, std::ios_base::out);
        }
        return out_stream.is_open();
    }

    void output_line(LogLevel, std::string string)
    {
        if (out_stream.is_open()) {
            out_stream << string << std::endl;
        }
    }

    virtual ~FileLogger()
    {
        out_stream.close();
    }
};

class StdoutLogger : public LoggerBackend {
public:
    StdoutLogger()
    {
    }
    virtual ~StdoutLogger() {}
    void output_line(LogLevel level, std::string string)
    {
        if (level == LogLevel::ERROR_LEVEL) {
            std::cerr << string << std::endl;
        } else {
            std::cout << string << std::endl;
        }
    }
};

class Logger {
    LoggerBackend *p_backend;
    LoggerBackend *p_default_backend;
public:
    Logger()
    {
        p_backend = NULL;
        p_default_backend = new StdoutLogger();
    }

    Logger(LoggerBackend *p_backend)
    {
        p_default_backend = NULL;
        set_backend(p_backend);
    }

    void set_backend(LoggerBackend *p_backend)
    {
        this->p_backend = p_backend;
    }

    void output_line(LogLevel level, const char *p_file, int line, std::string string)
    {
        std::string time_str;
        time_t raw_time;
        time(&raw_time);
        time_str = ctime(&raw_time);
        //without the newline character
        std::string output_string = time_str.substr(0, time_str.size() - 1);

        output_string += " - ";

        switch (level) {
        case LogLevel::TRACE_LEVEL:
            output_string += "TRACE:  ";
            break;
        case LogLevel::DEBUG_LEVEL:
            output_string += "DEBUG:  ";
            break;
        case LogLevel::INFO_LEVEL:
            output_string += "INFO :  ";
            break;
        case LogLevel::WARN_LEVEL:
            output_string += "WARN :  ";
            break;
        case LogLevel::ERROR_LEVEL:
        default:
            output_string += "ERROR:  ";
            break;
        }

        output_string += string;

        output_string += " (";
        output_string += p_file;
        output_string += " [";
        output_string += std::to_string(line);
        output_string += "])";

        if (p_backend != NULL) {
            p_backend->output_line(level, output_string.c_str());
        } else {
            p_default_backend->output_line(level, output_string.c_str());
        }
    }

    ~Logger()
    {
        if (p_default_backend != NULL) delete p_default_backend;
        if (p_backend != NULL) delete p_backend;
    }
};

#endif /* LOGGER_H_ */
