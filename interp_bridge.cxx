#include "ASTImportSource.h"
#include <dlfcn.h>
#include <chrono>
#include <fstream>
#include <unistd.h>

void (*dump_)(const char *);

void memory_status(){

  std::string line;

  std::ifstream stat_in_statm("/proc/self/statm");
  std::cout << "=======================================" << std::endl;
  std::cout << "/proc/self/statm" << std::endl;
  std::cout << "=======================================" << std::endl;
  if(stat_in_statm.is_open()){
    long rss, size;
    stat_in_statm >> size >> rss;
    stat_in_statm.close();
    long page_size = sysconf(_SC_PAGESIZE) /1024;
    std::cout << "Virtual Memory used : " << size * page_size << " kB " << std::endl;
    std::cout << "RSS : " << rss * page_size << " kB" << std::endl;

  }else std::cout << "Unable to open /proc/self/statm file";

  std::ifstream stat_in_smaps("/proc/self/smaps");
  std::cout << "=======================================" << std::endl;
  std::cout << "/proc/self/smaps" << std::endl;
  std::cout << "=======================================" << std::endl;
  if(stat_in_smaps.is_open()){
    while ( getline (stat_in_smaps,line) )
    {
      if(line.find("[heap]") != std::string::npos){
        std::cout << line << '\n';
        for(int i = 0; i<15; i++ ){
          getline (stat_in_smaps,line);
          std::cout << line << '\n';
        }
      }
    }
    stat_in_smaps.close();
  }else std::cout << "Unable to open /proc/self/smaps file";
}

int main(int argc, char** argv) {

  if (void *sym = dlsym(0, "igprof_dump_now")) {
    dump_ = __extension__ (void(*)(const char *)) sym;
  } else {
    dump_=0;
    std::cout << "Heap profile requested but application is not"
    << " currently being profiled with igprof" << std::endl;
  }

  std::chrono::time_point<std::chrono::system_clock> start, end;
  std::chrono::duration<double, std::milli> elapsed_seconds;

  start = std::chrono::system_clock::now();
  /*-----------------------I1 Creation----------------------------------*/
  cling::Interpreter I1(argc, argv, LLVMRESDIR);
  /*--------------------------------------------------------------------*/
  end = std::chrono::system_clock::now();
  elapsed_seconds = end-start;

  std::cout << "Time for the creation of the I1: "
  << elapsed_seconds.count() << "ms\n"
  << std::endl;

  /* IgProf: I1 creation */
  if (dump_) {
    std::string heapfile = "heapfile-I1-creation.mp.gz";
    std::ofstream igprof_out;
    igprof_out.open(heapfile);
    dump_(heapfile.c_str());
  }

  start = std::chrono::system_clock::now();
  cling::IncrementalExecutor* I1IncrExecutor = I1.getIncrementalExecutor();
  {
    /*-----------------------I2 Creation----------------------------*/
    cling::Interpreter I2(&I1, argc, argv, LLVMRESDIR);

    clang::TranslationUnitDecl *translationUnitDeclI2 =
      I2.getCI()->getASTContext().getTranslationUnitDecl();

    if(I1IncrExecutor)
      I2.setExternalIncrementalExecutor(I1IncrExecutor);

    cling::Interpreter *I1_p = &I1;
    cling::Interpreter *I2_p = &I2;

    //1. Create an external source from the information from the
    //first interpreter
    ASTImportSource *myExternalSource = new ASTImportSource(I1_p, I2_p);

    clang::ASTContext& tuI2ASTContext = I2.getCI()->getASTContext();

    llvm::IntrusiveRefCntPtr <clang::ExternalASTSource>
      astContextExternalSource(myExternalSource);

    tuI2ASTContext.ExternalSource.resetWithoutRelease();
    tuI2ASTContext.setExternalSource(astContextExternalSource);
    /* Inform the Translation Unit Decl of I2 that it has to
     * search somewhere else to find the declarations. */
    translationUnitDeclI2->setHasExternalVisibleStorage();

    /*--------------------------------------------------------------*/

    end = std::chrono::system_clock::now();
    elapsed_seconds = end-start;
    std::cout << "time for the creation of the I2: "
    << elapsed_seconds.count() << "ms\n"
    << std::endl;

    /* IgProf: I2 creation */
    if (dump_) {
      std::string heapfile = "heapfile-I2-creation.mp.gz";
      std::ofstream igprof_out;
      igprof_out.open(heapfile);
      dump_(heapfile.c_str());
    }
    /*---------------------------TESTS-------------------------*/
    I1.declare("#include <iostream>");
    I1.declare("#include <vector>");
    I1.declare("#include <map>");

    I1.declare("template<typename X, typename Y> void func(X x, Y y) {};");
    I1.declare("template<typename X> void func(X x, int y);");
    I1.declare("int x;");
    I2.execute("x=2;");
    I2.execute("func(2,2);");

    I1.declare("template <class T>\n"
                 "class mypair {\n"
                 "    T values [2];\n"
                 "  public:\n"
                 "    mypair (T first, T second)\n"
                 "    {\n"
                 "      values[0]=first; values[1]=second;\n"
                 "    }\n"
                 "};");

    I2.declare("mypair<int> *myobject;");

    I1.declare("std::vector<int> myvec;");

    I2.declare("int x;");
    I1.declare("extern \"C\"{"
                   "void foo(){"
                    "std::cout << \"Interp1::foo \" << std::endl;"
                    "static int i = 0;"
                    "std::cout << ++i << std::endl;"
                   "}"
                  "}");

    I1.execute("foo();");
    I2.execute("foo();");
    I1.execute("foo();");

     I1.declare("void hello(){ std::cout << \"hello(void)\" << std::endl; }");
     I1.declare("void hello(int i){ std::cout << \"hello(int)\" << std::endl; }");
     I1.execute("hello(8999)");
     I2.execute("hello()");
     I2.execute("hello(8);");

    I1.declare("void I1func(int i){ std::cout << \"I1::func, i = \" << i << std::endl;}");
    I1.execute("I1func(1);");
    I2.execute("I1func(2);");

    start = std::chrono::system_clock::now();
    I1.echo("4433");
    end = std::chrono::system_clock::now();
    elapsed_seconds = end-start;

    std::cout << "Time for the echo of I1: "
    << elapsed_seconds.count() << "ms\n"
    << std::endl;

    /* IgProf: I1 echo */
    if (dump_) {
      std::string heapfile = "heapfile-I1-echo.mp.gz";
      std::ofstream igprof_out;
      igprof_out.open(heapfile);
      dump_(heapfile.c_str());
    }

    start = std::chrono::system_clock::now();
    I2.echo("3");
    end = std::chrono::system_clock::now();
    elapsed_seconds = end-start;

    std::cout << "Time for the echo of I2: "
    << elapsed_seconds.count() << "ms\n"
    << std::endl;

    /* IgProf: I2 echo */
    if (dump_) {
      std::string heapfile = "heapfile-I2-echo.mp.gz";
      std::ofstream igprof_out;
      igprof_out.open(heapfile);
      dump_(heapfile.c_str());
    }
    //I2.declare("std::map<int,int> m_map;");
    //I2.declare("std::vector<int> myvec2;");
  } // I2 destructed here
  return 0;
}