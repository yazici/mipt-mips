/*
 * kernel.h - base kernel class
 * @author Vyacheslav Kompan kompan.vo@phystech.edu
 * Copyright 2018 MIPT-MIPS
 */

#ifndef KERNEL_H
#define KERNEL_H

#include <func_sim/traps/trap.h>
#include <infra/exception.h>
#include <memory/memory.h>
#include <simulator.h>

#include <iostream>
#include <memory>

struct BadInputValue final : Exception {
    explicit BadInputValue( const std::string& msg) : Exception( "Bad input value", msg) {}
};

class Operation;

class Kernel {
public:
    static std::shared_ptr<Kernel> create_configured_kernel();
    static std::shared_ptr<Kernel> create_dummy_kernel();
    static std::shared_ptr<Kernel> create_mars_kernel();
    static std::shared_ptr<Kernel> create_mars_kernel( std::istream& cin, std::ostream& cout, std::ostream& cerr);

    virtual void set_simulator( const std::shared_ptr<CPUModel>& s) = 0;
    virtual void connect_memory( std::shared_ptr<FuncMemory> m) = 0;
    virtual void connect_exception_handler() = 0;
    virtual void add_replica_simulator( const std::shared_ptr<CPUModel>& s) = 0;
    virtual void add_replica_memory( const std::shared_ptr<FuncMemory>& s) = 0;
    virtual void load_file( const std::string& name) = 0;

    virtual Trap execute() = 0;
    Trap execute_interactive();
    void handle_instruction( Operation* instr);

    Kernel() = default;
    virtual ~Kernel() = default;
    Kernel( const Kernel&) = delete;
    Kernel( Kernel&&) = delete;
    Kernel& operator=( const Kernel&) = delete;
    Kernel& operator=( Kernel&&) = delete;

    int get_exit_code() const { return exit_code; }
    Addr get_start_pc() const { return start_pc; }

protected:
    int exit_code = 0;
    Addr start_pc = 0;
};

#endif //KERNEL_H
