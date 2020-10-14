include("keybinding.jl");
include("../keysym.jl")

const CLIB = "./juliawm.so"

function setup()
    ccall((:setup, CLIB), Cvoid, ())
end

function run(cmd::String)
    return ccall((:run, CLIB), Cvoid, (Cstring,), cmd)
end

function cleanup()
    ccall((:cleanup, CLIB), Cvoid, ())
end

function updateConfig()
    ccall((:updateConfig, CLIB), Cvoid, ())
end

function main()
    startup_cmd = ""
    c = 0

    if length(ARGS) > 0
        startup_cmd = ARGS[1]
    end
    println(startup_cmd)

    if isempty(ENV["XDG_RUNTIME_DIR"])
        println("XDG_RUNTIME_DIR must be set")
    end

    updateConfig()
    setup()
    run(startup_cmd)
    cleanup()
end

main()
