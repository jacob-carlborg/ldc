#ifndef _llvmd_gen_logger_h_
#define _llvmd_gen_logger_h_

#include <iosfwd>

namespace llvm {
    class Type;
    class Value;
}

#ifndef IS_PRINTF
# ifdef __GNUC__
#  define IS_PRINTF(FMTARG) __attribute((__format__ (__printf__, (FMTARG), (FMTARG)+1) ))
# else
#  define IS_PRINTF(FMTARG)
# endif
#endif

struct Loc;

class Stream {
    std::ostream* OS;
    
public:
    Stream() : OS(0) {}
    Stream(std::ostream* S) : OS(S) {}
    Stream(std::ostream& S) : OS(&S) {}
    
    Stream operator << (std::ios_base &(*Func)(std::ios_base&)) {
      if (OS) *OS << Func;
      return *this;
    }
    
    Stream operator << (std::ostream &(*Func)(std::ostream&)) {
      if (OS) *OS << Func;
      return *this;
    }
    
    template<typename Ty>
    Stream& operator << (const Ty& Thing) {
        if (OS)
            Writer<Ty, sizeof(sfinae_bait(Thing))>::write(*OS, Thing);
        return *this;
    }
    
private:
    // Implementation details to treat llvm::Value, llvm::Type and their
    // subclasses specially (to pretty-print types).
    
    static void writeType(std::ostream& OS, const llvm::Type& Ty);
    static void writeValue(std::ostream& OS, const llvm::Value& Ty);
    
    template<typename Ty, int N> friend struct Writer;
    // error: function template partial specialization is not allowed
    // So I guess type partial specialization + member function will have to do...
    template<typename Ty, int N>
    struct Writer {
        static void write(std::ostream& OS, const Ty& Thing) {
            OS << Thing;
        }
    };
    
    template<typename Ty>
    struct Writer<Ty, 1> {
        static void write(std::ostream& OS, const llvm::Type& Thing) {
            Stream::writeType(OS, Thing);
        }
        static void write(std::ostream& OS, const llvm::Value& Thing) {
            Stream::writeValue(OS, Thing);
        }
    };
    
    // NOT IMPLEMENTED
    char sfinae_bait(const llvm::Type&);
    char sfinae_bait(const llvm::Value&);
    short sfinae_bait(...);
};

namespace Logger
{
    void indent();
    void undent();
    Stream cout();
    void println(const char* fmt, ...) IS_PRINTF(1);
    void print(const char* fmt, ...) IS_PRINTF(1);
    void enable();
    void disable();
    bool enabled();

    void attention(Loc loc, const char* fmt, ...) IS_PRINTF(2);

    struct LoggerScope
    {
        LoggerScope()
        {
            Logger::indent();
        }
        ~LoggerScope()
        {
            Logger::undent();
        }
    };
}

#define LOG_SCOPE    Logger::LoggerScope _logscope;

#define IF_LOG       if (Logger::enabled())

#endif

