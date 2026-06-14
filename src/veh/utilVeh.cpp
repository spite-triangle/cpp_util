
#include "utilVeh.h"

#include <csignal>
#include <iostream>
#include <unordered_set>

namespace 
{
/* ========== hook =========== */
extern __declspec(noinline) void __cdecl report_securityfailure(ULONG failure_code){
    util::CoreDump::instance()->_handle(util::CoreDump::REPORT_SECURITY);
    __fastfail(failure_code);
}

extern __declspec(noreturn) void __cdecl report_rangecheckfailure(){
    util::CoreDump::instance()->_handle(util::CoreDump::REPORT_RANGE_CHECK);
    __fastfail(3080);
}

extern __declspec(noreturn) void __cdecl report_gsfailure(uintptr_t stack_cookie){
    util::CoreDump::instance()->_handle(util::CoreDump::REPORT_GS);
    __fastfail(3090);
}

/* hook 函数 */
static BOOL PreventFuncall(void *oldfun,void *newfun) {
    void* pOrgEntry = oldfun;
    if (pOrgEntry == nullptr) return FALSE;
    DWORD dwOldProtect = 0;SIZE_T jmpSize = 5;
#ifdef _M_X64
    jmpSize = 13;
#endif
    BOOL bProt = VirtualProtect(pOrgEntry, jmpSize,
        PAGE_EXECUTE_READWRITE, &dwOldProtect);
    BYTE newJump[20];
    void* pNewFunc = newfun;
#ifdef _M_IX86
    DWORD dwOrgEntryAddr = (DWORD)pOrgEntry;
    dwOrgEntryAddr += jmpSize;
    DWORD dwNewEntryAddr = (DWORD)pNewFunc;
    DWORD dwRelativeAddr = dwNewEntryAddr - dwOrgEntryAddr;
    newJump[0] = 0xE9;
    memcpy(&newJump[1], &dwRelativeAddr, sizeof(pNewFunc));
#elif _M_X64
    newJump[0] = 0x49;newJump[1] = 0xBB;
    memcpy(&newJump[2], &pNewFunc, sizeof(pNewFunc));
    newJump[10] = 0x41;newJump[11] = 0xFF;newJump[12] = 0xE3;
#endif
    SIZE_T bytesWritten;
    BOOL bRet = WriteProcessMemory(GetCurrentProcess(),pOrgEntry, newJump, jmpSize, &bytesWritten);

    if (bProt != FALSE) {
        DWORD dwBuf;VirtualProtect(pOrgEntry, jmpSize, dwOldProtect, &dwBuf);
    }
    return bRet;
}


static int HookFailure(){
    auto  flag = PreventFuncall(&__report_gsfailure, &report_gsfailure);
    if(flag == FALSE){
        std::cerr << "failed to hook __report_gsfailure" << std::endl;
        return flag;
    }

    flag = PreventFuncall(&__report_rangecheckfailure, &report_rangecheckfailure);
    if(flag == FALSE){ 
        std::cerr << "failed to hook __report_rangecheckfailure" << std::endl;
        return flag;
    }

    flag = PreventFuncall(&__report_securityfailure, &report_securityfailure);
    if(flag == FALSE){
         std::cerr << "failed to hook report_securityfailure" << std::endl;
         return flag;
    }

    return flag;
}

/* ========== hook =========== */


/* ========== signal =========== */

static void SignalHandle(int nSignal){
    std::cerr << "receive signal "<< nSignal << std::endl;

    switch (nSignal)
    {
    case SIGILL: util::CoreDump::instance()->_handle(util::CoreDump::SIG_ILL); break;
    case SIGFPE: util::CoreDump::instance()->_handle(util::CoreDump::SIG_FPE); break;
    case SIGSEGV: util::CoreDump::instance()->_handle(util::CoreDump::SIG_SEGV); break;
    case SIGTERM: util::CoreDump::instance()->_handle(util::CoreDump::SIG_TERM); break;
    case SIGABRT: util::CoreDump::instance()->_handle(util::CoreDump::SIG_ABRT); break; 
    default:
        break;
    }
}

static void HookSignal(){
    _set_abort_behavior(0, _WRITE_ABORT_MSG);

    std::signal(SIGILL, SignalHandle);
    std::signal(SIGFPE, SignalHandle);
    std::signal(SIGSEGV, SignalHandle);
    std::signal(SIGTERM, SignalHandle);
    std::signal(SIGABRT, SignalHandle);
}

/* ========== signal =========== */

/* ============= VEH ============ */

// https://learn.microsoft.com/zh-cn/windows/win32/api/winnt/ns-winnt-exception_record
// 要捕获的异常
static std::unordered_set<DWORD> g_setExceptionCode{
    EXCEPTION_ACCESS_VIOLATION,         //线程尝试从虚拟地址读取或写入其没有相应访问权限的虚拟地址。
    EXCEPTION_ARRAY_BOUNDS_EXCEEDED,    //线程尝试访问超出边界且基础硬件支持边界检查的数组元素。
    EXCEPTION_DATATYPE_MISALIGNMENT,    //线程尝试读取或写入在不提供对齐的硬件上未对齐的数据。 例如，16 位值必须在 2 字节边界上对齐;4 字节边界上的 32 位值等。
    EXCEPTION_FLT_DENORMAL_OPERAND,     //浮点运算中的一个操作数是反常运算。 非规范值太小，无法表示为标准浮点值。
    EXCEPTION_FLT_DIVIDE_BY_ZERO,       //线程尝试将浮点值除以 0 的浮点除数。
    EXCEPTION_FLT_STACK_CHECK,          //堆栈因浮点运算而溢出或下溢。
    EXCEPTION_ILLEGAL_INSTRUCTION,      //线程尝试执行无效指令。
    EXCEPTION_IN_PAGE_ERROR,            //线程尝试访问不存在的页面，但系统无法加载该页。 例如，如果在通过网络运行程序时网络连接断开，则可能会发生此异常。
    EXCEPTION_INT_DIVIDE_BY_ZERO,       //线程尝试将整数值除以零的整数除数。
    EXCEPTION_INVALID_DISPOSITION,      //异常处理程序向异常调度程序返回了无效处置。 使用高级语言（如 C）的程序员不应遇到此异常。
    EXCEPTION_NONCONTINUABLE_EXCEPTION, //线程尝试在发生不可连续的异常后继续执行。
    EXCEPTION_PRIV_INSTRUCTION,         //线程尝试执行在当前计算机模式下不允许其操作的指令。
    EXCEPTION_STACK_OVERFLOW,           //线程占用了其堆栈。
    STATUS_HEAP_CORRUPTION,             //堆栈被破坏
    STATUS_STACK_BUFFER_OVERRUN,
    STATUS_INVALID_CRUNTIME_PARAMETER,  
    STATUS_ASSERTION_FAILURE
};

static LONG ExceptionFilter(_EXCEPTION_POINTERS * pInfo){
    auto code = pInfo->ExceptionRecord->ExceptionCode;

    if(g_setExceptionCode.count(code) > 0){
        std::cerr << "catch system exception and code is " << code << std::endl;
        util::CoreDump::instance()->_filter(pInfo);
    }

    return EXCEPTION_CONTINUE_SEARCH;
}

/* ============= VEH ============ */

} // namespace 

namespace util{

int CoreDump::registerFailureHandler(const FcnHanle_t & fcn){
    if(HookFailure() == FALSE){
        return -1;
    }
    HookSignal();

    if(fcn){
        m_fcnHandle = fcn;
    }
    return 0;
}

int CoreDump::registerExceptionFilter(const FcnFilter_t & fcn){
    AddVectoredContinueHandler(0, ExceptionFilter);
    if(fcn){
        m_fcnFilter = fcn;
    }
    return 0;
}

void CoreDump::_handle(const FAILURE_ERROR_E &enType)
{
    if(m_fcnHandle){
        m_fcnHandle(enType);
    }
}

void CoreDump::_filter(const _EXCEPTION_POINTERS *pInfo)
{
    if(m_fcnFilter){
        m_fcnFilter(pInfo);
    }
}

} // namespace util
