#!/usr/bin/env bats

load "$TESTDIR/utils.sh"

make_input() {
    "${PYTHON:-python3}" "$BATS_TEST_DIRNAME/make_input.py" "$BATS_TEST_DIRNAME" "$1" "$2" "$3" "$4" "$5" "$6"
}

make_all_inputs() {
    make_input t_input.peg "t_imp_0" "" "t_imp_0.peg" "" "t_dir_1/t_imp_4.peg"
    make_input t_imp_0.peg "t_imp_1" "" "t_imp_1.peg" "" ""
    make_input t_imp_1.peg "t_imp_2" "" "" "" "t_imp_2.peg"
    make_input t_dir_0/t_imp_2.peg "t_imp_3" "" "../t_dir_0/t_imp_3.peg" "" "t_imp_3.peg"
    make_input t_dir_0/t_imp_3.peg "t_imp_4" "" "" "" ""
    make_input t_dir_1/t_imp_4.peg "t_imp_5" "" "t_dir_1_0/../t_imp_5.peg" "" "t_imp_5.peg"
    make_input t_dir_1/t_imp_5.peg "t_imp_6" "t_imp_6.peg" "../t_dir_1/t_dir_1_0/t_imp_7.peg" "" ""
    make_input t_dir_1/t_dir_1_0/t_imp_6.peg "t_imp_7" "" "" "" ""
    make_input t_dir_1/t_dir_1_0/t_imp_7.peg "t_imp_8" "t_imp_8.peg" "../t_imp_a.peg" "" ""
    make_input t_dir_2/t_imp_8.peg "t_imp_9" "" "../t_dir_2/t_imp_9.peg" "" "t_imp_9.peg"
    make_input t_dir_2/t_imp_9.peg "t_imp_a" "" "" "" ""
    make_input t_dir_3/t_imp_a.peg "t_imp_b" "" "t_dir_3_0/../t_imp_b.peg" "" "t_imp_b.peg"
    make_input t_dir_3/t_imp_b.peg "t_imp_c" "t_imp_c.peg" "../t_dir_3/t_dir_3_0/t_imp_d.peg" "" ""
    make_input t_dir_3/t_dir_3_0/t_imp_c.peg "t_imp_d" "" "" "" ""
    make_input t_dir_3/t_dir_3_0/t_imp_d.peg "" "" "" "t_imp_c.peg" ""
}

@test "Testing import.d - generation" {
    "$PACKCC" --debug -o "$BATS_TEST_DIRNAME/t_parser" "$BATS_TEST_DIRNAME/reference.peg" > "$BATS_TEST_DIRNAME/t_expected.txt" 2>&1
    make_all_inputs
    export PCC_IMPORT_PATH=$BATS_TEST_DIRNAME/t_dir_2:$BATS_TEST_DIRNAME/t_dir_3/t_dir_3_0
    run "$PACKCC" -I "$BATS_TEST_DIRNAME/t_dir_0" -I "$BATS_TEST_DIRNAME/t_dir_1/t_dir_1_0" --debug --lines -o "$BATS_TEST_DIRNAME/parser" "$BATS_TEST_DIRNAME/t_input.peg" 2>&1
    check_output "$BATS_TEST_DIRNAME/t_expected.txt"
}

@test "Testing import.d - header ordering" {
    ! in_header ":t_input_EARLYSOURCE:"
    ! in_header ":t_imp_0_EARLYSOURCE:"
    ! in_header ":t_imp_1_EARLYSOURCE:"
    ! in_header ":t_imp_2_EARLYSOURCE:"
    ! in_header ":t_imp_3_EARLYSOURCE:"
    ! in_header ":t_imp_4_EARLYSOURCE:"
    ! in_header ":t_imp_5_EARLYSOURCE:"
    ! in_header ":t_imp_6_EARLYSOURCE:"
    ! in_header ":t_imp_7_EARLYSOURCE:"
    ! in_header ":t_imp_8_EARLYSOURCE:"
    ! in_header ":t_imp_9_EARLYSOURCE:"
    ! in_header ":t_imp_a_EARLYSOURCE:"
    ! in_header ":t_imp_b_EARLYSOURCE:"
    ! in_header ":t_imp_c_EARLYSOURCE:"
    ! in_header ":t_imp_d_EARLYSOURCE:"
    ! in_header ":t_input_SOURCE:"
    ! in_header ":t_imp_0_SOURCE:"
    ! in_header ":t_imp_1_SOURCE:"
    ! in_header ":t_imp_2_SOURCE:"
    ! in_header ":t_imp_3_SOURCE:"
    ! in_header ":t_imp_4_SOURCE:"
    ! in_header ":t_imp_5_SOURCE:"
    ! in_header ":t_imp_6_SOURCE:"
    ! in_header ":t_imp_7_SOURCE:"
    ! in_header ":t_imp_8_SOURCE:"
    ! in_header ":t_imp_9_SOURCE:"
    ! in_header ":t_imp_a_SOURCE:"
    ! in_header ":t_imp_b_SOURCE:"
    ! in_header ":t_imp_c_SOURCE:"
    ! in_header ":t_imp_d_SOURCE:"
    ! in_header ":t_input_CODE:"
    ! in_header ":t_imp_0_CODE:"
    ! in_header ":t_imp_1_CODE:"
    ! in_header ":t_imp_2_CODE:"
    ! in_header ":t_imp_3_CODE:"
    ! in_header ":t_imp_4_CODE:"
    ! in_header ":t_imp_5_CODE:"
    ! in_header ":t_imp_6_CODE:"
    ! in_header ":t_imp_7_CODE:"
    ! in_header ":t_imp_8_CODE:"
    ! in_header ":t_imp_9_CODE:"
    ! in_header ":t_imp_a_CODE:"
    ! in_header ":t_imp_b_CODE:"
    ! in_header ":t_imp_c_CODE:"
    ! in_header ":t_imp_d_CODE:"
    L000=$(get_line ":t_input_EARLYHEADER:" parser.h)
    L001=$(get_line ":t_input_EARLYCOMMON:" parser.h)
    L002=$(get_line ":t_imp_0_EARLYHEADER:" parser.h)
    L003=$(get_line ":t_imp_0_EARLYCOMMON:" parser.h)
    L004=$(get_line ":t_imp_1_EARLYHEADER:" parser.h)
    L005=$(get_line ":t_imp_1_EARLYCOMMON:" parser.h)
    L006=$(get_line ":t_imp_2_EARLYHEADER:" parser.h)
    L007=$(get_line ":t_imp_2_EARLYCOMMON:" parser.h)
    L008=$(get_line ":t_imp_3_EARLYHEADER:" parser.h)
    L009=$(get_line ":t_imp_3_EARLYCOMMON:" parser.h)
    L010=$(get_line ":t_imp_4_EARLYHEADER:" parser.h)
    L011=$(get_line ":t_imp_4_EARLYCOMMON:" parser.h)
    L012=$(get_line ":t_imp_5_EARLYHEADER:" parser.h)
    L013=$(get_line ":t_imp_5_EARLYCOMMON:" parser.h)
    L014=$(get_line ":t_imp_6_EARLYHEADER:" parser.h)
    L015=$(get_line ":t_imp_6_EARLYCOMMON:" parser.h)
    L016=$(get_line ":t_imp_7_EARLYHEADER:" parser.h)
    L017=$(get_line ":t_imp_7_EARLYCOMMON:" parser.h)
    L018=$(get_line ":t_imp_8_EARLYHEADER:" parser.h)
    L019=$(get_line ":t_imp_8_EARLYCOMMON:" parser.h)
    L020=$(get_line ":t_imp_9_EARLYHEADER:" parser.h)
    L021=$(get_line ":t_imp_9_EARLYCOMMON:" parser.h)
    L022=$(get_line ":t_imp_a_EARLYHEADER:" parser.h)
    L023=$(get_line ":t_imp_a_EARLYCOMMON:" parser.h)
    L024=$(get_line ":t_imp_b_EARLYHEADER:" parser.h)
    L025=$(get_line ":t_imp_b_EARLYCOMMON:" parser.h)
    L026=$(get_line ":t_imp_c_EARLYHEADER:" parser.h)
    L027=$(get_line ":t_imp_c_EARLYCOMMON:" parser.h)
    L028=$(get_line ":t_imp_d_EARLYHEADER:" parser.h)
    L029=$(get_line ":t_imp_d_EARLYCOMMON:" parser.h)
    L099=$(get_line "#ifndef PCC_INCLUDED_PARSER_H" parser.h)
    L100=$(get_line ":t_input_HEADER:" parser.h)
    L101=$(get_line ":t_input_COMMON:" parser.h)
    L102=$(get_line ":t_imp_0_HEADER:" parser.h)
    L103=$(get_line ":t_imp_0_COMMON:" parser.h)
    L104=$(get_line ":t_imp_1_HEADER:" parser.h)
    L105=$(get_line ":t_imp_1_COMMON:" parser.h)
    L106=$(get_line ":t_imp_2_HEADER:" parser.h)
    L107=$(get_line ":t_imp_2_COMMON:" parser.h)
    L108=$(get_line ":t_imp_3_HEADER:" parser.h)
    L109=$(get_line ":t_imp_3_COMMON:" parser.h)
    L110=$(get_line ":t_imp_4_HEADER:" parser.h)
    L111=$(get_line ":t_imp_4_COMMON:" parser.h)
    L112=$(get_line ":t_imp_5_HEADER:" parser.h)
    L113=$(get_line ":t_imp_5_COMMON:" parser.h)
    L114=$(get_line ":t_imp_6_HEADER:" parser.h)
    L115=$(get_line ":t_imp_6_COMMON:" parser.h)
    L116=$(get_line ":t_imp_7_HEADER:" parser.h)
    L117=$(get_line ":t_imp_7_COMMON:" parser.h)
    L118=$(get_line ":t_imp_8_HEADER:" parser.h)
    L119=$(get_line ":t_imp_8_COMMON:" parser.h)
    L120=$(get_line ":t_imp_9_HEADER:" parser.h)
    L121=$(get_line ":t_imp_9_COMMON:" parser.h)
    L122=$(get_line ":t_imp_a_HEADER:" parser.h)
    L123=$(get_line ":t_imp_a_COMMON:" parser.h)
    L124=$(get_line ":t_imp_b_HEADER:" parser.h)
    L125=$(get_line ":t_imp_b_COMMON:" parser.h)
    L126=$(get_line ":t_imp_c_HEADER:" parser.h)
    L127=$(get_line ":t_imp_c_COMMON:" parser.h)
    L128=$(get_line ":t_imp_d_HEADER:" parser.h)
    L129=$(get_line ":t_imp_d_COMMON:" parser.h)
    L199=$(get_line "pcc_create" parser.h)
    [ "$L000" -lt "$L001" ]
    [ "$L001" -lt "$L002" ]
    [ "$L002" -lt "$L003" ]
    [ "$L003" -lt "$L004" ]
    [ "$L004" -lt "$L005" ]
    [ "$L005" -lt "$L006" ]
    [ "$L006" -lt "$L007" ]
    [ "$L007" -lt "$L008" ]
    [ "$L008" -lt "$L009" ]
    [ "$L009" -lt "$L010" ]
    [ "$L010" -lt "$L011" ]
    [ "$L011" -lt "$L012" ]
    [ "$L012" -lt "$L013" ]
    [ "$L013" -lt "$L014" ]
    [ "$L014" -lt "$L015" ]
    [ "$L015" -lt "$L016" ]
    [ "$L016" -lt "$L017" ]
    [ "$L017" -lt "$L018" ]
    [ "$L018" -lt "$L019" ]
    [ "$L019" -lt "$L020" ]
    [ "$L020" -lt "$L021" ]
    [ "$L021" -lt "$L022" ]
    [ "$L022" -lt "$L023" ]
    [ "$L023" -lt "$L024" ]
    [ "$L024" -lt "$L025" ]
    [ "$L025" -lt "$L026" ]
    [ "$L026" -lt "$L027" ]
    [ "$L027" -lt "$L028" ]
    [ "$L028" -lt "$L029" ]
    [ "$L029" -lt "$L099" ]
    [ "$L099" -lt "$L100" ]
    [ "$L100" -lt "$L101" ]
    [ "$L101" -lt "$L102" ]
    [ "$L102" -lt "$L103" ]
    [ "$L103" -lt "$L104" ]
    [ "$L104" -lt "$L105" ]
    [ "$L105" -lt "$L106" ]
    [ "$L106" -lt "$L107" ]
    [ "$L107" -lt "$L108" ]
    [ "$L108" -lt "$L109" ]
    [ "$L109" -lt "$L110" ]
    [ "$L110" -lt "$L111" ]
    [ "$L111" -lt "$L112" ]
    [ "$L112" -lt "$L113" ]
    [ "$L113" -lt "$L114" ]
    [ "$L114" -lt "$L115" ]
    [ "$L115" -lt "$L116" ]
    [ "$L116" -lt "$L117" ]
    [ "$L117" -lt "$L118" ]
    [ "$L118" -lt "$L119" ]
    [ "$L119" -lt "$L120" ]
    [ "$L120" -lt "$L121" ]
    [ "$L121" -lt "$L122" ]
    [ "$L122" -lt "$L123" ]
    [ "$L123" -lt "$L124" ]
    [ "$L124" -lt "$L125" ]
    [ "$L125" -lt "$L126" ]
    [ "$L126" -lt "$L127" ]
    [ "$L127" -lt "$L128" ]
    [ "$L128" -lt "$L129" ]
    [ "$L129" -lt "$L199" ]
}

@test "Testing import.d - source ordering" {
    ! in_source ":t_input_EARLYHEADER:"
    ! in_source ":t_imp_0_EARLYHEADER:"
    ! in_source ":t_imp_1_EARLYHEADER:"
    ! in_source ":t_imp_2_EARLYHEADER:"
    ! in_source ":t_imp_3_EARLYHEADER:"
    ! in_source ":t_imp_4_EARLYHEADER:"
    ! in_source ":t_imp_5_EARLYHEADER:"
    ! in_source ":t_imp_6_EARLYHEADER:"
    ! in_source ":t_imp_7_EARLYHEADER:"
    ! in_source ":t_imp_8_EARLYHEADER:"
    ! in_source ":t_imp_9_EARLYHEADER:"
    ! in_source ":t_imp_a_EARLYHEADER:"
    ! in_source ":t_imp_b_EARLYHEADER:"
    ! in_source ":t_imp_c_EARLYHEADER:"
    ! in_source ":t_imp_d_EARLYHEADER:"
    ! in_source ":t_input_HEADER:"
    ! in_source ":t_imp_0_HEADER:"
    ! in_source ":t_imp_1_HEADER:"
    ! in_source ":t_imp_2_HEADER:"
    ! in_source ":t_imp_3_HEADER:"
    ! in_source ":t_imp_4_HEADER:"
    ! in_source ":t_imp_5_HEADER:"
    ! in_source ":t_imp_6_HEADER:"
    ! in_source ":t_imp_7_HEADER:"
    ! in_source ":t_imp_8_HEADER:"
    ! in_source ":t_imp_9_HEADER:"
    ! in_source ":t_imp_a_HEADER:"
    ! in_source ":t_imp_b_HEADER:"
    ! in_source ":t_imp_c_HEADER:"
    ! in_source ":t_imp_d_HEADER:"
    L000=$(get_line ":t_input_EARLYSOURCE:" parser.c)
    L001=$(get_line ":t_input_EARLYCOMMON:" parser.c)
    L002=$(get_line ":t_imp_0_EARLYSOURCE:" parser.c)
    L003=$(get_line ":t_imp_0_EARLYCOMMON:" parser.c)
    L004=$(get_line ":t_imp_1_EARLYSOURCE:" parser.c)
    L005=$(get_line ":t_imp_1_EARLYCOMMON:" parser.c)
    L006=$(get_line ":t_imp_2_EARLYSOURCE:" parser.c)
    L007=$(get_line ":t_imp_2_EARLYCOMMON:" parser.c)
    L008=$(get_line ":t_imp_3_EARLYSOURCE:" parser.c)
    L009=$(get_line ":t_imp_3_EARLYCOMMON:" parser.c)
    L010=$(get_line ":t_imp_4_EARLYSOURCE:" parser.c)
    L011=$(get_line ":t_imp_4_EARLYCOMMON:" parser.c)
    L012=$(get_line ":t_imp_5_EARLYSOURCE:" parser.c)
    L013=$(get_line ":t_imp_5_EARLYCOMMON:" parser.c)
    L014=$(get_line ":t_imp_6_EARLYSOURCE:" parser.c)
    L015=$(get_line ":t_imp_6_EARLYCOMMON:" parser.c)
    L016=$(get_line ":t_imp_7_EARLYSOURCE:" parser.c)
    L017=$(get_line ":t_imp_7_EARLYCOMMON:" parser.c)
    L018=$(get_line ":t_imp_8_EARLYSOURCE:" parser.c)
    L019=$(get_line ":t_imp_8_EARLYCOMMON:" parser.c)
    L020=$(get_line ":t_imp_9_EARLYSOURCE:" parser.c)
    L021=$(get_line ":t_imp_9_EARLYCOMMON:" parser.c)
    L022=$(get_line ":t_imp_a_EARLYSOURCE:" parser.c)
    L023=$(get_line ":t_imp_a_EARLYCOMMON:" parser.c)
    L024=$(get_line ":t_imp_b_EARLYSOURCE:" parser.c)
    L025=$(get_line ":t_imp_b_EARLYCOMMON:" parser.c)
    L026=$(get_line ":t_imp_c_EARLYSOURCE:" parser.c)
    L027=$(get_line ":t_imp_c_EARLYCOMMON:" parser.c)
    L028=$(get_line ":t_imp_d_EARLYSOURCE:" parser.c)
    L029=$(get_line ":t_imp_d_EARLYCOMMON:" parser.c)
    L098=$(get_line '#include <stdio.h>' parser.c)
    L099=$(get_line '#include "parser.h"' parser.c)
    L100=$(get_line ":t_input_SOURCE:" parser.c)
    L101=$(get_line ":t_input_COMMON:" parser.c)
    L102=$(get_line ":t_imp_0_SOURCE:" parser.c)
    L103=$(get_line ":t_imp_0_COMMON:" parser.c)
    L104=$(get_line ":t_imp_1_SOURCE:" parser.c)
    L105=$(get_line ":t_imp_1_COMMON:" parser.c)
    L106=$(get_line ":t_imp_2_SOURCE:" parser.c)
    L107=$(get_line ":t_imp_2_COMMON:" parser.c)
    L108=$(get_line ":t_imp_3_SOURCE:" parser.c)
    L109=$(get_line ":t_imp_3_COMMON:" parser.c)
    L110=$(get_line ":t_imp_4_SOURCE:" parser.c)
    L111=$(get_line ":t_imp_4_COMMON:" parser.c)
    L112=$(get_line ":t_imp_5_SOURCE:" parser.c)
    L113=$(get_line ":t_imp_5_COMMON:" parser.c)
    L114=$(get_line ":t_imp_6_SOURCE:" parser.c)
    L115=$(get_line ":t_imp_6_COMMON:" parser.c)
    L116=$(get_line ":t_imp_7_SOURCE:" parser.c)
    L117=$(get_line ":t_imp_7_COMMON:" parser.c)
    L118=$(get_line ":t_imp_8_SOURCE:" parser.c)
    L119=$(get_line ":t_imp_8_COMMON:" parser.c)
    L120=$(get_line ":t_imp_9_SOURCE:" parser.c)
    L121=$(get_line ":t_imp_9_COMMON:" parser.c)
    L122=$(get_line ":t_imp_a_SOURCE:" parser.c)
    L123=$(get_line ":t_imp_a_COMMON:" parser.c)
    L124=$(get_line ":t_imp_b_SOURCE:" parser.c)
    L125=$(get_line ":t_imp_b_COMMON:" parser.c)
    L126=$(get_line ":t_imp_c_SOURCE:" parser.c)
    L127=$(get_line ":t_imp_c_COMMON:" parser.c)
    L128=$(get_line ":t_imp_d_SOURCE:" parser.c)
    L129=$(get_line ":t_imp_d_COMMON:" parser.c)
    L198=$(get_line "#define PCC_BUFFER_MIN_SIZE" parser.c)
    L199=$(get_line "pcc_destroy" parser.c)
    L200=$(get_line ":t_imp_3_CODE:" parser.c)
    L201=$(get_line ":t_imp_2_CODE:" parser.c)
    L202=$(get_line ":t_imp_1_CODE:" parser.c)
    L203=$(get_line ":t_imp_0_CODE:" parser.c)
    L204=$(get_line ":t_imp_6_CODE:" parser.c)
    L205=$(get_line ":t_imp_9_CODE:" parser.c)
    L206=$(get_line ":t_imp_8_CODE:" parser.c)
    L207=$(get_line ":t_imp_c_CODE:" parser.c)
    L208=$(get_line ":t_imp_d_CODE:" parser.c)
    L209=$(get_line ":t_imp_b_CODE:" parser.c)
    L210=$(get_line ":t_imp_a_CODE:" parser.c)
    L211=$(get_line ":t_imp_7_CODE:" parser.c)
    L212=$(get_line ":t_imp_5_CODE:" parser.c)
    L213=$(get_line ":t_imp_4_CODE:" parser.c)
    L214=$(get_line ":t_input_CODE:" parser.c)
    [ "$L000" -lt "$L001" ]
    [ "$L001" -lt "$L002" ]
    [ "$L002" -lt "$L003" ]
    [ "$L003" -lt "$L004" ]
    [ "$L004" -lt "$L005" ]
    [ "$L005" -lt "$L006" ]
    [ "$L006" -lt "$L007" ]
    [ "$L007" -lt "$L008" ]
    [ "$L008" -lt "$L009" ]
    [ "$L009" -lt "$L010" ]
    [ "$L010" -lt "$L011" ]
    [ "$L011" -lt "$L012" ]
    [ "$L012" -lt "$L013" ]
    [ "$L013" -lt "$L014" ]
    [ "$L014" -lt "$L015" ]
    [ "$L015" -lt "$L016" ]
    [ "$L016" -lt "$L017" ]
    [ "$L017" -lt "$L018" ]
    [ "$L018" -lt "$L019" ]
    [ "$L019" -lt "$L020" ]
    [ "$L020" -lt "$L021" ]
    [ "$L021" -lt "$L022" ]
    [ "$L022" -lt "$L023" ]
    [ "$L023" -lt "$L024" ]
    [ "$L024" -lt "$L025" ]
    [ "$L025" -lt "$L026" ]
    [ "$L026" -lt "$L027" ]
    [ "$L027" -lt "$L028" ]
    [ "$L028" -lt "$L029" ]
    [ "$L029" -lt "$L098" ]
    [ "$L098" -lt "$L099" ]
    [ "$L099" -lt "$L100" ]
    [ "$L100" -lt "$L101" ]
    [ "$L101" -lt "$L102" ]
    [ "$L102" -lt "$L103" ]
    [ "$L103" -lt "$L104" ]
    [ "$L104" -lt "$L105" ]
    [ "$L105" -lt "$L106" ]
    [ "$L106" -lt "$L107" ]
    [ "$L107" -lt "$L108" ]
    [ "$L108" -lt "$L109" ]
    [ "$L109" -lt "$L110" ]
    [ "$L110" -lt "$L111" ]
    [ "$L111" -lt "$L112" ]
    [ "$L112" -lt "$L113" ]
    [ "$L113" -lt "$L114" ]
    [ "$L114" -lt "$L115" ]
    [ "$L115" -lt "$L116" ]
    [ "$L116" -lt "$L117" ]
    [ "$L117" -lt "$L118" ]
    [ "$L118" -lt "$L119" ]
    [ "$L119" -lt "$L120" ]
    [ "$L120" -lt "$L121" ]
    [ "$L121" -lt "$L122" ]
    [ "$L122" -lt "$L123" ]
    [ "$L123" -lt "$L124" ]
    [ "$L124" -lt "$L125" ]
    [ "$L125" -lt "$L126" ]
    [ "$L126" -lt "$L127" ]
    [ "$L127" -lt "$L128" ]
    [ "$L128" -lt "$L129" ]
    [ "$L129" -lt "$L198" ]
    [ "$L198" -lt "$L199" ]
    [ "$L199" -lt "$L200" ]
    [ "$L200" -lt "$L201" ]
    [ "$L201" -lt "$L202" ]
    [ "$L202" -lt "$L203" ]
    [ "$L203" -lt "$L204" ]
    [ "$L204" -lt "$L205" ]
    [ "$L205" -lt "$L206" ]
    [ "$L206" -lt "$L207" ]
    [ "$L207" -lt "$L208" ]
    [ "$L208" -lt "$L209" ]
    [ "$L209" -lt "$L210" ]
    [ "$L210" -lt "$L211" ]
    [ "$L211" -lt "$L212" ]
    [ "$L212" -lt "$L213" ]
    [ "$L213" -lt "$L214" ]
}

check_line_number() {
    "${PYTHON:-python3}" "$BATS_TEST_DIRNAME/check_line_number.py" "$1" "$BATS_TEST_DIRNAME/$2"
}

check_line_number_pre() {
    "${PYTHON:-python3}" "$BATS_TEST_DIRNAME/check_line_number.py" --only-pre "$1" "$BATS_TEST_DIRNAME/$2"
}

@test "Testing import.d - header line numbers" {
    check_line_number ":t_input_EARLYHEADER:" parser.h
    check_line_number ":t_input_EARLYCOMMON:" parser.h
    check_line_number ":t_imp_0_EARLYHEADER:" parser.h
    check_line_number ":t_imp_0_EARLYCOMMON:" parser.h
    check_line_number ":t_imp_1_EARLYHEADER:" parser.h
    check_line_number ":t_imp_1_EARLYCOMMON:" parser.h
    check_line_number ":t_imp_2_EARLYHEADER:" parser.h
    check_line_number ":t_imp_2_EARLYCOMMON:" parser.h
    check_line_number ":t_imp_3_EARLYHEADER:" parser.h
    check_line_number ":t_imp_3_EARLYCOMMON:" parser.h
    check_line_number ":t_imp_4_EARLYHEADER:" parser.h
    check_line_number ":t_imp_4_EARLYCOMMON:" parser.h
    check_line_number ":t_imp_5_EARLYHEADER:" parser.h
    check_line_number ":t_imp_5_EARLYCOMMON:" parser.h
    check_line_number ":t_imp_6_EARLYHEADER:" parser.h
    check_line_number ":t_imp_6_EARLYCOMMON:" parser.h
    check_line_number ":t_imp_7_EARLYHEADER:" parser.h
    check_line_number ":t_imp_7_EARLYCOMMON:" parser.h
    check_line_number ":t_imp_8_EARLYHEADER:" parser.h
    check_line_number ":t_imp_8_EARLYCOMMON:" parser.h
    check_line_number ":t_imp_9_EARLYHEADER:" parser.h
    check_line_number ":t_imp_9_EARLYCOMMON:" parser.h
    check_line_number ":t_imp_a_EARLYHEADER:" parser.h
    check_line_number ":t_imp_a_EARLYCOMMON:" parser.h
    check_line_number ":t_imp_b_EARLYHEADER:" parser.h
    check_line_number ":t_imp_b_EARLYCOMMON:" parser.h
    check_line_number ":t_imp_c_EARLYHEADER:" parser.h
    check_line_number ":t_imp_c_EARLYCOMMON:" parser.h
    check_line_number ":t_imp_d_EARLYHEADER:" parser.h
    check_line_number ":t_imp_d_EARLYCOMMON:" parser.h
    check_line_number ":t_input_HEADER:" parser.h
    check_line_number ":t_input_COMMON:" parser.h
    check_line_number ":t_imp_0_HEADER:" parser.h
    check_line_number ":t_imp_0_COMMON:" parser.h
    check_line_number ":t_imp_1_HEADER:" parser.h
    check_line_number ":t_imp_1_COMMON:" parser.h
    check_line_number ":t_imp_2_HEADER:" parser.h
    check_line_number ":t_imp_2_COMMON:" parser.h
    check_line_number ":t_imp_3_HEADER:" parser.h
    check_line_number ":t_imp_3_COMMON:" parser.h
    check_line_number ":t_imp_4_HEADER:" parser.h
    check_line_number ":t_imp_4_COMMON:" parser.h
    check_line_number ":t_imp_5_HEADER:" parser.h
    check_line_number ":t_imp_5_COMMON:" parser.h
    check_line_number ":t_imp_6_HEADER:" parser.h
    check_line_number ":t_imp_6_COMMON:" parser.h
    check_line_number ":t_imp_7_HEADER:" parser.h
    check_line_number ":t_imp_7_COMMON:" parser.h
    check_line_number ":t_imp_8_HEADER:" parser.h
    check_line_number ":t_imp_8_COMMON:" parser.h
    check_line_number ":t_imp_9_HEADER:" parser.h
    check_line_number ":t_imp_9_COMMON:" parser.h
    check_line_number ":t_imp_a_HEADER:" parser.h
    check_line_number ":t_imp_a_COMMON:" parser.h
    check_line_number ":t_imp_b_HEADER:" parser.h
    check_line_number ":t_imp_b_COMMON:" parser.h
    check_line_number ":t_imp_c_HEADER:" parser.h
    check_line_number ":t_imp_c_COMMON:" parser.h
    check_line_number ":t_imp_d_HEADER:" parser.h
    check_line_number ":t_imp_d_COMMON:" parser.h
}

@test "Testing import.d - source line numbers" {
    check_line_number ":t_input_EARLYSOURCE:" parser.c
    check_line_number ":t_input_EARLYCOMMON:" parser.c
    check_line_number ":t_imp_0_EARLYSOURCE:" parser.c
    check_line_number ":t_imp_0_EARLYCOMMON:" parser.c
    check_line_number ":t_imp_1_EARLYSOURCE:" parser.c
    check_line_number ":t_imp_1_EARLYCOMMON:" parser.c
    check_line_number ":t_imp_2_EARLYSOURCE:" parser.c
    check_line_number ":t_imp_2_EARLYCOMMON:" parser.c
    check_line_number ":t_imp_3_EARLYSOURCE:" parser.c
    check_line_number ":t_imp_3_EARLYCOMMON:" parser.c
    check_line_number ":t_imp_4_EARLYSOURCE:" parser.c
    check_line_number ":t_imp_4_EARLYCOMMON:" parser.c
    check_line_number ":t_imp_5_EARLYSOURCE:" parser.c
    check_line_number ":t_imp_5_EARLYCOMMON:" parser.c
    check_line_number ":t_imp_6_EARLYSOURCE:" parser.c
    check_line_number ":t_imp_6_EARLYCOMMON:" parser.c
    check_line_number ":t_imp_7_EARLYSOURCE:" parser.c
    check_line_number ":t_imp_7_EARLYCOMMON:" parser.c
    check_line_number ":t_imp_8_EARLYSOURCE:" parser.c
    check_line_number ":t_imp_8_EARLYCOMMON:" parser.c
    check_line_number ":t_imp_9_EARLYSOURCE:" parser.c
    check_line_number ":t_imp_9_EARLYCOMMON:" parser.c
    check_line_number ":t_imp_a_EARLYSOURCE:" parser.c
    check_line_number ":t_imp_a_EARLYCOMMON:" parser.c
    check_line_number ":t_imp_b_EARLYSOURCE:" parser.c
    check_line_number ":t_imp_b_EARLYCOMMON:" parser.c
    check_line_number ":t_imp_c_EARLYSOURCE:" parser.c
    check_line_number ":t_imp_c_EARLYCOMMON:" parser.c
    check_line_number ":t_imp_d_EARLYSOURCE:" parser.c
    check_line_number ":t_imp_d_EARLYCOMMON:" parser.c
    check_line_number ":t_input_SOURCE:" parser.c
    check_line_number ":t_input_COMMON:" parser.c
    check_line_number ":t_imp_0_SOURCE:" parser.c
    check_line_number ":t_imp_0_COMMON:" parser.c
    check_line_number ":t_imp_1_SOURCE:" parser.c
    check_line_number ":t_imp_1_COMMON:" parser.c
    check_line_number ":t_imp_2_SOURCE:" parser.c
    check_line_number ":t_imp_2_COMMON:" parser.c
    check_line_number ":t_imp_3_SOURCE:" parser.c
    check_line_number ":t_imp_3_COMMON:" parser.c
    check_line_number ":t_imp_4_SOURCE:" parser.c
    check_line_number ":t_imp_4_COMMON:" parser.c
    check_line_number ":t_imp_5_SOURCE:" parser.c
    check_line_number ":t_imp_5_COMMON:" parser.c
    check_line_number ":t_imp_6_SOURCE:" parser.c
    check_line_number ":t_imp_6_COMMON:" parser.c
    check_line_number ":t_imp_7_SOURCE:" parser.c
    check_line_number ":t_imp_7_COMMON:" parser.c
    check_line_number ":t_imp_8_SOURCE:" parser.c
    check_line_number ":t_imp_8_COMMON:" parser.c
    check_line_number ":t_imp_9_SOURCE:" parser.c
    check_line_number ":t_imp_9_COMMON:" parser.c
    check_line_number ":t_imp_a_SOURCE:" parser.c
    check_line_number ":t_imp_a_COMMON:" parser.c
    check_line_number ":t_imp_b_SOURCE:" parser.c
    check_line_number ":t_imp_b_COMMON:" parser.c
    check_line_number ":t_imp_c_SOURCE:" parser.c
    check_line_number ":t_imp_c_COMMON:" parser.c
    check_line_number ":t_imp_d_SOURCE:" parser.c
    check_line_number ":t_imp_d_COMMON:" parser.c
    check_line_number_pre ":t_imp_3_CODE:" parser.c
    check_line_number_pre ":t_imp_2_CODE:" parser.c
    check_line_number_pre ":t_imp_1_CODE:" parser.c
    check_line_number_pre ":t_imp_0_CODE:" parser.c
    check_line_number_pre ":t_imp_6_CODE:" parser.c
    check_line_number_pre ":t_imp_9_CODE:" parser.c
    check_line_number_pre ":t_imp_8_CODE:" parser.c
    check_line_number_pre ":t_imp_c_CODE:" parser.c
    check_line_number_pre ":t_imp_d_CODE:" parser.c
    check_line_number_pre ":t_imp_b_CODE:" parser.c
    check_line_number_pre ":t_imp_a_CODE:" parser.c
    check_line_number_pre ":t_imp_7_CODE:" parser.c
    check_line_number_pre ":t_imp_5_CODE:" parser.c
    check_line_number_pre ":t_imp_4_CODE:" parser.c
    check_line_number_pre ":t_input_CODE:" parser.c
}
