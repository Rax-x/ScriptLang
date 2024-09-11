#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "../include/parser.h"
#include "../include/compiler.h"
#include "../include/vm.h"

using scriptlang::compiler::Compiler;
using scriptlang::parser::Parser;
using scriptlang::error::BasicErrorReporter;
using scriptlang::ast::printer::AstPrettyPrinter;
using scriptlang::runtime::VM;
using scriptlang::types::Byte;

constexpr Byte EXECUTE = 0b0000'0000;
constexpr Byte DUMP_AST = 0b0000'0001;
constexpr Byte DUMP_BYTECODE = 0b0000'0010;

static VM vm;

static auto readSourceFromFile(const char* path) -> std::string {

    std::ifstream stream(path);
    
    if(!stream.is_open()) {
        std::cout << "An error occurred during reading the file!\n";
        std::exit(EXIT_FAILURE);
    }

    std::size_t size;
    
    stream.seekg(0, std::ios::end);
    size = stream.tellg();
    stream.seekg(0, std::ios::beg);
    
    std::string source(size, '\0');
    stream.read(&source[0], size);

    return source;
}

static auto runCode(const std::string& source, std::uint8_t flags) -> void {
    scriptlang::runtime::ObjectFunction function;

    {
        auto reporter = std::make_unique<BasicErrorReporter>();
        Parser parser(source, reporter.get());

        auto ast = parser.parseSoruce();
        if(reporter->hadError()){

            for(const auto& error : reporter->errors()){
                std::cout << error << '\n';
            }

            return;
        }

        if(flags & DUMP_AST){
            AstPrettyPrinter printer(std::cout);
            printer.print(ast);
        }
    
        reporter->reset();

        Compiler compiler(Compiler::FunctionType::Script, reporter.get(), flags & DUMP_BYTECODE);
        function = compiler.compile(ast);
        
        if(reporter->hadError()){
            for(const auto& error : reporter->errors()){
                std::cout << error << '\n';
            }        

            return;
        }
    }

    if(flags == EXECUTE){
        vm.execute(&function);
    }
}

static auto printReplCommands() -> void {
    std::cout << "\nREPL commands:\n"
              << "\t.exit\tExits from REPL mode.\n"
              << "\t.help\tPrints all REPL commands.\n"
              << "\t.ast-dump\tToggle AST dump.\n"
              << "\t.bytecode-dump\tToggle bytecode dump.\n";
}

static auto repl() -> void {

    bool astDump = false;
    bool bytecodeDump = false;
    std::string line;
    
    while(true){
        std::cout << "scriptlang >> ";
        
        if(!std::getline(std::cin, line)){
            std::cout << "An error occurred during reading from standard input!";
            std::exit(EXIT_FAILURE);
        }

        if(line.empty()) continue;
        
        if(line == ".exit"){
            break;
        } else if(line == ".help"){
            printReplCommands();
            continue;
        } else if(line == ".ast-dump"){
            astDump = !astDump;
            std::cout << "AST Dump " << (astDump ? "activated" : "disabled") << ".\n";
            continue;
        } else if(line == ".bytecode-dump"){
            bytecodeDump = !bytecodeDump;
            std::cout << "Bytecode Dump " << (bytecodeDump ? "activated" : "disabled") << ".\n";
            continue;
        }

        std::uint8_t flags = EXECUTE;
        
        if(astDump) flags |= DUMP_AST;
        if(bytecodeDump) flags |= DUMP_BYTECODE;

        runCode(line, flags);
    }
}

static inline auto runFromFile(const char* filename, bool dump) -> void {
    std::string source = readSourceFromFile(filename);
    runCode(source, dump ? (DUMP_AST | DUMP_BYTECODE) : EXECUTE);
}

static auto usage(const char* program) -> void {

    std::cout << "Usage: " << program << " [Options] [Source files]\n\n"
        << "Options:\n"
        << "\t--help\tPrint the usage of the program.\n"
        << "\t--dump\tPrint the generated AST and Bytecode.\n";

    printReplCommands();

    std::cout << "\n\n";
}

auto main(int argc, char** argv) -> int {

    bool shouldDump = false;

    if(argc == 1){
        repl();
        return 1;
    }

    char** args = argv + 1;
    for(args = argv + 1; *args != argv[argc]; args++){

        if(**args != '-') break;

        if(std::strcmp(*args, "--help") == 0 ||
           std::strcmp(*args, "-h") == 0) {

            usage(argv[0]);
            std::exit(EXIT_SUCCESS);
        } else if(std::strcmp(*args, "--dump") == 0){
            shouldDump = true;
        }
    }

    runFromFile(*args, shouldDump);
    
    return 0;
}
