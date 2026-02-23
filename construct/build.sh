current_path=`pwd`
source ${current_path}/script/env.sh

params=$@

pre_compiler() {
    rm -rf ${OUTPUT_PATH}
}

pre_compiler
for param in ${params[@]}
do
    make ${param}
done