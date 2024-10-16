name : Ewan Jee
netid : ej307

name : Chul Seung Lee
netid : cl1550

Test Plans:

Test 1:

    This test runs mysh with the file name test1.txt

        This text file consists of following contents:
            cd subdir
            echo hello
            cd subsubdir
            pwd
            cd directory_that_does_not_exist
            cd ../..
            pwd
            exit
        
        The first line changes the current working directory to the "subdir" directory
        The second line displays the string hello to the mysh terminal
        The third line changes the current working directory to the "subsubdir"
        The fourth command prints the current working directory to the console.
        The fifth command attempts to change to a directory that does not exist. Therefore, it will cause an error
        The sixth command moves up two levels in the directory hierarchy
        Finally, the exit command is used to exit the shell.

Test 2:

    This test runs mysh with the file name test2.txt

        This text file consists of following contents:

            cat test1.txt
            ls *.txt
            echo "Hello, World!" > textTemp.txt
            ls textTemp.txt
            which ls
            ls *.txt | wc -l
            exit
        
        The first command shows the contents of the file "test1.txt" in this current folder
        The second command lists all files in the current directory with a ".txt" extension using the ls command.
        The third command uses the echo command to output the string "Hello, World!" and then writes the content to the file "textTemp.txt."
        The fourth command confirms if the textTemp.txt that we just made really exists clearly or not.
        The fifth command displays the path to the 'ls' command
        The sixth command uses ls with the -all option to list all files and directories with details, and then pipes (|) the output to grep to filter lines containing files with a ".txt" extension.
        The final command will quit the mysh

Test 4:

    While previous tests run with a test file and this is for Batch Mode, we tested Interactive Mode by ourselves.
    Followings are some commands we've tried:
    cat > tt.txt
    hello
    ^D # --> this means ctrl + D to put above input into the text file and quit this current process
    cat < tt.txt
    # Since we input hello to tt.txt and the expected contents of tt.txt is hello, the real contents is the same as the expected.
    # We found that the command cat with '>' or '<' runs well.

Test 5:

    The extension of the file for test 3 is shell.

        This shell file will test conditionals.
        The file consists of following contents:

            ls
            then echo true
            cd
            else echo false
        
        We know that the command, 'ls' executes well meaning this command is true to execute. Therefore, mysh will execute 'echo true'
        In addition, we set cd without additional information after cd doesn't execute meaning this command has wrong format to execute. Therefore mysh will execute 'echo false'

