
To build the executable, following steps are needed:

* Change current working directory to the main source directory located on the memory medium:
```
  cd "interpreter/"
```

* Create the build directory and enter it:
```
  mkdir -p "build/" && cd "build/"
```

* Run *CMake* to generate makefiles:
```
  cmake ..
```

* Run *make* to compile the source codes:
```
  make
```

* Sometimes, the first build fails because of some *CMake* dependencies issues.
If it happened rerun *CMake* and *make* again.
```
  cmake .. && make
```

The executable allows to perform all experiments that were presented in this diploma thesis:

* To perform simple scheduler performance experiment:
```
  ./build/interpreter --test-scheduler <messages_count> <variables_count>
```
Here the *<messages_count\>* and the *<variables_count\>* are integer parameters.

* To perform server-client application test:
```
  ./build/interpreter --test-server <duration>
```
Here the *<duration\>* is an integer parameter.

* To perform GUI application test:
```
  ./build/interpreter --test-gui <duration>
```
Here the *<duration\>* is an integer parameter.

* To interpret specific source code file:
```
  ./build/interpreter <path_to_file> <arg>
```
Here the *<path_to_file\>* is a path of the file that will be interpreted and
the *<arg\>* is a string argument that will be passed to the main function.
