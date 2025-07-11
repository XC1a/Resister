set -eo pipefail

INCLUDE="."

gcc -I$INCLUDE -g -DuECC_ENABLE_VLI_API=1 -DTESTING=1 -DuECC_WORD_SIZE=4 test.c -o testprogram
