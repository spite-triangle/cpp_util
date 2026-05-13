#ifndef UTILEXCEPTION_HPP
#define UTILEXCEPTION_HPP


#include <string>
#include <sstream>
#include <exception>

#ifndef DISABLE_STACK_TRACE
// https://github.com/bombela/backward-cpp
#include "backward.hpp"
#endif

namespace util
{

// Exception hierarchy (Python-like exceptions mapped to C++)
// Skipped: BaseExceptionGroup, GeneratorExit, KeyboardInterrupt, SystemExit (Python-specific)

#ifndef DISABLE_STACK_TRACE

class BaseException : public std::exception{

public:
    BaseException() : std::exception(), m_message("") {
        m_stack.load_here();
    }
    BaseException(const std::string & msg): std::exception(), m_message(msg) {
        m_stack.load_here();
    }
    virtual ~BaseException() = default;
    
    virtual const char* what() const noexcept {
        return m_message.c_str();
    }

    const backward::StackTrace & stack() const {
        return m_stack;
    }
    

    std::string detail() const{

        std::stringstream ss;
        ss << "error: " << m_message << std::endl;

        backward::Printer p;
        p.reverse = true;
        p.print(m_stack, ss);
        return ss.str();
    }

protected:
    std::string m_message;
    backward::StackTrace m_stack;
};

#else

class BaseException : public std::exception{

public:
    BaseException() : std::exception(), m_message("") {
    }
    BaseException(const std::string & msg): std::exception(), m_message(msg) {
    }
    virtual ~BaseException() = default;
    
    virtual const char* what() const noexcept {
        return m_message.c_str();
    }

    std::string detail() const{

        std::stringstream ss;
        ss << "error: " << m_message << std::endl;
        ss << "[WARNNING] if you want to print stack info, you should remove `DISABLE_STACK_TRACE` micro define." << std::endl;
        return ss.str();
    }
 
protected:
    std::string m_message;
};

#endif

class Exception : public BaseException{
public:
    Exception(const std::string & msg): BaseException(msg) {}
};


// ArithmeticError - implemented as BaseException (no std::arithmetic_error exists)
class ArithmeticError : public Exception {
public:
    ArithmeticError(const std::string& msg) : Exception(msg) {}
};

class FloatingPointError : public ArithmeticError {
public:
    FloatingPointError(const std::string& msg) : ArithmeticError(msg) {}
};

class OverflowError : public ArithmeticError {
public:
    OverflowError(const std::string& msg) : ArithmeticError(msg){}
};

class ZeroDivisionError : public ArithmeticError {
public:
    ZeroDivisionError(const std::string& msg) : ArithmeticError(msg) {}
};

class AssertionError : public Exception {
public:
    AssertionError(const std::string& msg) : Exception(msg) {}
};

class AttributeError : public Exception {
public:
    AttributeError(const std::string& msg) : Exception(msg) {}
};

class BufferError : public Exception {
public:
    BufferError(const std::string& msg) : Exception(msg) {}
};

class EOFError : public Exception {
public:
    EOFError(const std::string& msg) : Exception(msg) {}
};

// ImportError
class ImportError : public Exception {
public:
    ImportError(const std::string& msg) : Exception(msg) {}
};

class ModuleNotFoundError : public ImportError {
public:
    ModuleNotFoundError(const std::string& msg) : ImportError(msg) {}
};

// LookupError
class LookupError : public Exception {
public:
    LookupError(const std::string& msg) : Exception(msg) {}
};

class IndexError : public LookupError {
public:
    IndexError(const std::string& msg) : LookupError(msg) {}
};

class KeyError : public LookupError {
public:
    KeyError(const std::string& msg) : LookupError(msg) {}
};

class MemoryError : public Exception {
public:
    MemoryError(const std::string& msg) : Exception(msg) {}
};

// NameError
class NameError : public Exception {
public:
    NameError(const std::string& msg) : Exception(msg) {}
};

class UnboundLocalError : public NameError {
public:
    UnboundLocalError(const std::string& msg) : NameError(msg) {}
};

// OSError - custom implementation (std::system_error requires error code)
class OSError : public Exception {
public:
    OSError(const std::string& msg) : Exception(msg) {}
};

class BlockingIOError : public OSError {
public:
    BlockingIOError(const std::string& msg) : OSError(msg) {}
};

class ChildProcessError : public OSError {
public:
    ChildProcessError(const std::string& msg) : OSError(msg) {}
};

// ConnectionError
class ConnectionError : public OSError {
public:
    ConnectionError(const std::string& msg) : OSError(msg) {}
};

class BrokenPipeError : public ConnectionError {
public:
    BrokenPipeError(const std::string& msg) : ConnectionError(msg) {}
};

class ConnectionAbortedError : public ConnectionError {
public:
    ConnectionAbortedError(const std::string& msg) : ConnectionError(msg) {}
};

class ConnectionRefusedError : public ConnectionError {
public:
    ConnectionRefusedError(const std::string& msg) : ConnectionError(msg) {}
};

class ConnectionResetError : public ConnectionError {
public:
    ConnectionResetError(const std::string& msg) : ConnectionError(msg) {}
};

class FileExistsError : public OSError {
public:
    FileExistsError(const std::string& msg) : OSError(msg) {}
};

class FileNotFoundError : public OSError {
public:
    FileNotFoundError(const std::string& msg) : OSError(msg) {}
};

class InterruptedError : public OSError {
public:
    InterruptedError(const std::string& msg) : OSError(msg) {}
};

class IsADirectoryError : public OSError {
public:
    IsADirectoryError(const std::string& msg) : OSError(msg) {}
};

class NotADirectoryError : public OSError {
public:
    NotADirectoryError(const std::string& msg) : OSError(msg) {}
};

class PermissionError : public OSError {
public:
    PermissionError(const std::string& msg) : OSError(msg) {}
};

class ProcessLookupError : public OSError {
public:
    ProcessLookupError(const std::string& msg) : OSError(msg) {}
};

class TimeoutError : public OSError {
public:
    TimeoutError(const std::string& msg) : OSError(msg) {}
};

class ReferenceError : public Exception {
public:
    ReferenceError(const std::string& msg) : Exception(msg) {}
};

// RuntimeError
class RuntimeError : public Exception{
public:
    RuntimeError(const std::string& msg) : Exception(msg){}
};

class NotImplementedError : public RuntimeError {
public:
    NotImplementedError(const std::string& msg) : RuntimeError(msg) {}
};

class RecursionError : public RuntimeError {
public:
    RecursionError(const std::string& msg) : RuntimeError(msg) {}
};

class StopIteration : public Exception {
public:
    StopIteration(const std::string& msg) : Exception(msg) {}
};

// SyntaxError
class SyntaxError : public Exception {
public:
    SyntaxError(const std::string& msg) : Exception(msg) {}
};

class IndentationError : public SyntaxError {
public:
    IndentationError(const std::string& msg) : SyntaxError(msg) {}
};

class TabError : public IndentationError {
public:
    TabError(const std::string& msg) : IndentationError(msg) {}
};

class SystemError : public Exception {
public:
    SystemError(const std::string& msg) : Exception(msg) {}
};

class TypeError : public Exception {
public:
    TypeError(const std::string& msg) : Exception(msg) {}
};

// ValueError
class ValueError : public Exception {
public:
    ValueError(const std::string& msg) : Exception(msg) {}
};

class UnicodeError : public ValueError {
public:
    UnicodeError(const std::string& msg) : ValueError(msg) {}
};

class UnicodeDecodeError : public UnicodeError {
public:
    UnicodeDecodeError(const std::string& msg) : UnicodeError(msg) {}
};

class UnicodeEncodeError : public UnicodeError {
public:
    UnicodeEncodeError(const std::string& msg) : UnicodeError(msg) {}
};

class UnicodeTranslateError : public UnicodeError {
public:
    UnicodeTranslateError(const std::string& msg) : UnicodeError(msg) {}
};

// Warning hierarchy (skipped as Python-specific)

} // namespace util

#endif /* UTILEXCEPTION_HPP */
