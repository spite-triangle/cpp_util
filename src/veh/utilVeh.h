#ifndef UTILVEH_HPP
#define UTILVEH_HPP

#include <windows.h>
#include <functional>

#include "utilCommon.hpp"

namespace util
{

class CoreDump: public Singleton<CoreDump>{
public:
    enum FAILURE_ERROR_E : int{
        REPORT_GS,
        REPORT_RANGE_CHECK,
        REPORT_SECURITY,
        SIG_ILL,
        SIG_FPE,
        SIG_SEGV,
        SIG_TERM,
        SIG_ABRT,
        PURE_CALL,
        INVALID_PARAMETER
    };

    using FcnHanle_t = std::function<void (const FAILURE_ERROR_E &)>;
    using FcnFilter_t = std::function<void (const _EXCEPTION_POINTERS *)>;
public:

    int registerFailureHandler(const FcnHanle_t & fcn);
    int registerExceptionFilter(const FcnFilter_t & fcn);

    /* internal call function*/
    void _handle(const FAILURE_ERROR_E & enType);
    void _filter(const _EXCEPTION_POINTERS * pInfo);
private:
    FcnHanle_t m_fcnHandle;
    FcnFilter_t m_fcnFilter;
};
    
} // namespace util


#endif /* UTILVEH_HPP */
