#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>

#define next argv[argp++]

void addArg(std::string &Out, const std::string &Arg) {
    Out.push_back(' ');
    Out.push_back('"');
    Out.append(Arg);
    Out.push_back('"');
}

int compileAndRun(std::string Compiler, const std::string &Source, const std::string &Output, const std::string &Tempfile) {
    addArg(Compiler, Source);
    if(system(Compiler.c_str())) return 1;

    std::string SCP = "$SCP";
    addArg(SCP, Output);
    SCP.append(" $REMOTE:/tmp/prg");
    //std::cout << SCP << std::endl;
    while(system(SCP.c_str()));

    std::string Exe = "$SSH $REMOTE /tmp/prg >";
    addArg(Exe, Tempfile);
    //std::cout << Exe << std::endl;
    system(Exe.c_str());

    return 0;
}

std::filesystem::path getFailsDir(const std::string &Source) {
    auto time = std::chrono::system_clock::now().time_since_epoch() / std::chrono::nanoseconds(1);
    std::filesystem::path PSource = Source;
    std::filesystem::path PDir = PSource.parent_path();
    std::filesystem::path FailsDir = PDir / "fails" / std::to_string(time);
    std::cout << FailsDir << std::endl;
    std::string mkdir = "mkdir -p " + FailsDir.string();
    system(mkdir.c_str());
    return FailsDir;
}

void saveFile(const std::filesystem::path &FailsDir, std::string Source) {
    std::filesystem::path PSource = Source;
    std::filesystem::path Curr = FailsDir / PSource.filename();
    std::string CP = "cp ";
    addArg(CP, Source);
    addArg(CP, Curr.string());
    std::cout << Curr << std::endl;
    system(CP.c_str());
}

int compare(const std::string &Tempfile1, const std::string &Tempfile2) {
    std::ifstream in1(Tempfile1);
    //in1.sync_with_stdio(0);
    //in1.tie(0);
    std::ifstream in2(Tempfile2);
    //in2.sync_with_stdio(0);
    //in2.tie(0);

    while(true) {
        char c1, c2;
        bool read1 = (bool)(in1 >> c1);
        bool read2 = (bool)(in2 >> c2);

        if(read1 != read2) return 1;
        if(!read1) return 0;
        if(c1 != c2) return 1;
    }
}

int main(int argc, char **argv) {
    int argp = 1;
    std::string Tempfile1 = next;
    std::string Tempfile2 = next;
    std::string Source1 = next;
    std::string Source2 = next;
    std::string Output = next;
    std::string Compiler = next;
    addArg(Compiler, "-o");
    addArg(Compiler, Output);

    int result =
        compileAndRun(Compiler, Source1, Output, Tempfile1)
        || compileAndRun(Compiler, Source2, Output, Tempfile2)
        || compare(Tempfile1, Tempfile2);
    if(result) {
        auto FailsDir = getFailsDir(Source1);
        saveFile(FailsDir, Source1);
        saveFile(FailsDir, Source2);
        saveFile(FailsDir, Tempfile1);
        saveFile(FailsDir, Tempfile2);
        std::cout << "FAIL" << std::endl;
    } else std::cout << "SUCCESS" << std::endl;
    return 0;
}
