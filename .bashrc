# Filename: .bashrc
# Description: This is the standard .bashrc for new named accounts.
#
# Please (DO NOT) edit this file unless you are sure of what you are doing.
# This file and other dotfiles have been written to work with each other.
# Any change that you are not sure off can break things in an unpredicatable
# ways.

[[ -e /share/b/bin/instructional.bashrc ]] && . /share/b/bin/instructional.bashrc


# For future sam:
# the ENABLE_SBT_THIN_CLIENT and conda things were added for cs152
# other things are in /scratch/$USER directory

export ENABLE_SBT_THIN_CLIENT=1
export CHIPYARDROOT=/scratch/$USER/chipyard
export BMARKS=$CHIPYARDROOT/generators/riscv-sodor/riscv-bmarks
export SCRIPTS=$CHIPYARDROOT/generators/riscv-sodor/scripts
source $CHIPYARDROOT/env.sh
export PS1='\[\e[01;32m\]\u@\h\[\e[00m\]:\[\e[01;34m\]\w\n\[\e[01:37m\]\[\e[00m\]\$ '

echo "Halfway 152 setup! doing conda things"

# >>> conda initialize >>>
# !! Contents within this block are managed by 'conda init' !!
__conda_setup="$('/scratch/cs199-aoq/conda/bin/conda' 'shell.bash' 'hook' 2> /dev/null)"
if [ $? -eq 0 ]; then
    eval "$__conda_setup"
else
    if [ -f "/scratch/cs199-aoq/conda/etc/profile.d/conda.sh" ]; then
        . "/scratch/cs199-aoq/conda/etc/profile.d/conda.sh"
    else
        export PATH="/scratch/cs199-aoq/conda/bin:$PATH"
    fi
fi
unset __conda_setup
# <<< conda initialize <<<


conda activate $CHIPYARDROOT/.conda-env/
echo "did 152 conda setup, now done"


