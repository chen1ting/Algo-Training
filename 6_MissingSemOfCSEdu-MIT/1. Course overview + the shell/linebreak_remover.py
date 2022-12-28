def linebreak_remover(txt):
    return txt.replace('\n', ' ')

print(linebreak_remover("""1. For this course, you need to be using a Unix shell like Bash or ZSH. If you are on Linux or
macOS, you don’t have to do anything special. If you are on Windows, you need to make sure
you are not running cmd.exe or PowerShell; you can use Windows Subsystem for
Linux (https://docs.microsoft.com/en-us/windows/wsl/) or a Linux virtual machine to use Unixstyle command-line tools. To make sure you’re running an appropriate shell, you can try the
command echo $SHELL . If it says something like /bin/bash or /usr/bin/zsh , that
means you’re running the right program."""
))