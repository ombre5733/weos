#!/bin/sh

if [ $# -eq 0 ]; then
  echo "Usage:"
  echo "  $0 <cmsisDir>"
  echo ""
  echo "<cmsisDir> is the directory of the CMSIS-RTX implementation"
  exit 1
fi

path="$1"

if [ ! -d "${path}/INC" ] ; then
  echo "The directory ${path}/INC does not exist"
  exit 1
fi
if [ ! -d "${path}/SRC" ] ; then
  echo "The directory ${path}/SRC does not exist"
  exit 1
fi

OUTFILE="cmsis_check.cpp"

cat /dev/null > ${OUTFILE}

echo "#include <cstddef>" >> ${OUTFILE}
echo "#include <iostream>" >> ${OUTFILE}
echo "#include <stdint.h>" >> ${OUTFILE}
echo "typedef uint8_t U8;" >> ${OUTFILE}
echo "typedef uint16_t U16;" >> ${OUTFILE}
echo "typedef uint32_t U32;" >> ${OUTFILE}
echo "using namespace std;" >> ${OUTFILE}
echo "" >> ${OUTFILE}

awk "/osCMSIS_RTX/" "${path}/INC/cmsis_os.h" >> ${OUTFILE}
echo "" >> ${OUTFILE}

echo "// ----" >> ${OUTFILE}
echo "// Mutex control block" >> ${OUTFILE}
echo "// ----" >> ${OUTFILE}
echo "// REQUIRE: sizeof(OS_MUCB) == 4 * sizeof(uint32_t)" >> ${OUTFILE}
echo "// REQUIRE: offsetof(OS_MUCB, cb_type) == 0" >> ${OUTFILE}
echo "// REQUIRE: offsetof(OS_MUCB, level) == 2" >> ${OUTFILE}
echo "// REQUIRE: offsetof(OS_MUCB, p_lnk) == 4" >> ${OUTFILE}
echo "// REQUIRE: offsetof(OS_MUCB, owner) == 8" >> ${OUTFILE}
echo "// REQUIRE: offsetof(OS_MUCB, p_mlnk) == 12" >> ${OUTFILE}
echo "// REQUIRE: MUCB == 3" >> ${OUTFILE}
echo "" >> ${OUTFILE}
awk "/OS_MUCB.*{/,/P_MUCB/" "${path}/SRC/rt_TypeDef.h" >> ${OUTFILE}
awk "/MUCB/" "${path}/SRC/rt_List.h" >> ${OUTFILE}
echo "" >> ${OUTFILE}

echo "// ----" >> ${OUTFILE}
echo "// Semaphore control block" >> ${OUTFILE}
echo "// ----" >> ${OUTFILE}
echo "// REQUIRE: sizeof(OS_SCB) == 4 * sizeof(uint16_t)" >> ${OUTFILE}
echo "// REQUIRE: offsetof(OS_SCB, cb_type) == 0" >> ${OUTFILE}
echo "// REQUIRE: offsetof(OS_SCB, tokens) == 2" >> ${OUTFILE}
echo "// REQUIRE: offsetof(OS_SCB, p_lnk) == 4" >> ${OUTFILE}
echo "" >> ${OUTFILE}
awk "/OS_SCB.*{/,/P_SCB/" "${path}/SRC/rt_TypeDef.h" >> ${OUTFILE}
echo "" >> ${OUTFILE}

echo "// ----" >> ${OUTFILE}
echo "// Task control block" >> ${OUTFILE}
echo "// ----" >> ${OUTFILE}
echo "// REQUIRE: offsetof(OS_TCB, priv_stack) == 38" >> ${OUTFILE}
echo "// REQUIRE: offsetof(OS_TCB, ptask) == 48" >> ${OUTFILE}
echo "" >> ${OUTFILE}
awk "/typedef.*FUNCP.*;/" "${path}/SRC/rt_TypeDef.h" >> ${OUTFILE}
awk "/OS_TCB.*{/,/P_TCB/" "${path}/SRC/rt_TypeDef.h" >> ${OUTFILE}
echo "" >> ${OUTFILE}

echo "// ----" >> ${OUTFILE}
echo "// Memory block" >> ${OUTFILE}
echo "// ----" >> ${OUTFILE}
echo "// REQUIRE: sizeof(OS_BM) == 3 * sizeof(uint32_t)" >> ${OUTFILE}
echo "// REQUIRE: offsetof(OS_BM, free) == 0" >> ${OUTFILE}
echo "// REQUIRE: offsetof(OS_BM, end) == 4" >> ${OUTFILE}
echo "// REQUIRE: offsetof(OS_BM, blk_size) == 8" >> ${OUTFILE}
awk "/OS_BM.*{/,/P_BM/" "${path}/SRC/rt_TypeDef.h" >> ${OUTFILE}
echo "" >> ${OUTFILE}

# Tests

printf "int main()\n{\n" >> ${OUTFILE}

printf '  static_assert(sizeof(void*) == sizeof(uint32_t), "Compile in 32-bit mode");\n' >> ${OUTFILE}

printf '  static_assert(sizeof(OS_MUCB) == 4 * sizeof(uint32_t), "Need correction");\n' >> ${OUTFILE}
printf '  static_assert(offsetof(OS_MUCB, cb_type) == 0, "Need correction");\n' >> ${OUTFILE}
printf '  static_assert(offsetof(OS_MUCB, level) == 2, "Need correction");\n' >> ${OUTFILE}
printf '  static_assert(offsetof(OS_MUCB, p_lnk) == 4, "Need correction");\n' >> ${OUTFILE}
printf '  static_assert(offsetof(OS_MUCB, owner) == 8, "Need correction");\n' >> ${OUTFILE}
printf '  static_assert(offsetof(OS_MUCB, p_mlnk) == 12, "Need correction");\n' >> ${OUTFILE}
printf '  static_assert(MUCB == 3, "Need correction");\n' >> ${OUTFILE}

printf '  static_assert(sizeof(OS_SCB) == 4 * sizeof(uint16_t), "Need correction");\n' >> ${OUTFILE}
printf '  static_assert(offsetof(OS_SCB, cb_type) == 0, "Need correction");\n' >> ${OUTFILE}
printf '  static_assert(offsetof(OS_SCB, tokens) == 2, "Need correction");\n' >> ${OUTFILE}
printf '  static_assert(offsetof(OS_SCB, p_lnk) == 4, "Need correction");\n' >> ${OUTFILE}

printf '  static_assert(offsetof(OS_TCB, priv_stack) == 38, "Need correction");\n' >> ${OUTFILE}
printf '  static_assert(offsetof(OS_TCB, ptask) == 48, "Need correction");\n' >> ${OUTFILE}

printf '  static_assert(sizeof(OS_BM) == 3 * sizeof(uint32_t), "Need correction");\n' >> ${OUTFILE}
printf '  static_assert(offsetof(OS_BM, free) == 0, "Need correction");\n' >> ${OUTFILE}
printf '  static_assert(offsetof(OS_BM, end) == 4, "Need correction");\n' >> ${OUTFILE}
printf '  static_assert(offsetof(OS_BM, blk_size) == 8, "Need correction");\n' >> ${OUTFILE}

printf "  cout << \"Generated `date`\" << endl;\n" >> ${OUTFILE}
printf '  cout << "CMSIS version: " << (osCMSIS_RTX >> 16) << "." << (osCMSIS_RTX & 0xFFFF) << endl;\n' >> ${OUTFILE}
printf '  cout << "All tests passed" << endl;\n' >> ${OUTFILE}
printf "}\n" >> ${OUTFILE}
