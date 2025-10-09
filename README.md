
# Osiris-src

This is the source tree for the Osiris kernel, an operating system aiming to be a modern, GNU-free Unix clone licensed under 0BSD.


## Building

First of all, you need to check dependencies and install them from your distro's package manager if needed.

```bash
  make check-deps
```

Now we can proceed onto building. Osiris has many targets. If you want to automatically build the complete iso, and run using qemu-system-x86_64, run:
```bash
  make all
```  
These instructions will most likely be changed in the near future.
