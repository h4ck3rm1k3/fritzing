// This file was generated by qlalr - DO NOT EDIT!
#include "gedaelementgrammar_p.h"

const char *const GedaElementGrammar::spell [] = {
  "end of file", 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 
#ifndef QLALR_NO_GEDAELEMENTGRAMMAR_DEBUG_INFO
"geda_element", "element_command", "element_arguments", "sub_element_groups", "element_flags", "description", "pcb_name", "value", 
  "mark_x", "mark_y", "text_x", "text_y", "text_direction", "text_scale", "text_flags", "sub_element_group", "pin_element", "pad_element", 
  "element_arc_element", "element_line_element", "pin_command", "pin_sequence", "pin_paren_sequence", "pin_bracket_sequence", "pin_arguments", "x", "y", "Thickness", 
  "Clearance", "Mask", "DrillHole", "Name", "pin_number", "Flags", "pad_command", "pad_sequence", "pad_paren_sequence", "pad_bracket_sequence", 
  "pad_arguments", "x1", "y1", "x2", "y2", "pad_number", "element_line_command", "element_line_sequence", "element_line_paren_sequence", "element_line_bracket_sequence", 
  "element_line_arguments", "element_arc_command", "element_arc_sequence", "element_arc_arguments", "element_arc_paren_sequence", "element_arc_bracket_sequence", "Width", "Height", "StartAngle", "Delta", 
  "string_value", "number_value", "$accept"
#endif // QLALR_NO_GEDAELEMENTGRAMMAR_DEBUG_INFO
};

const int GedaElementGrammar::lhs [] = {
  12, 14, 15, 15, 27, 27, 27, 27, 28, 33, 
  33, 34, 35, 36, 29, 47, 47, 48, 49, 50, 
  31, 57, 57, 58, 59, 60, 30, 62, 62, 64, 
  65, 63, 55, 37, 51, 53, 38, 52, 54, 39, 
  40, 41, 42, 43, 44, 45, 66, 67, 68, 69, 
  16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 
  26, 71, 70, 13, 32, 46, 56, 61, 72};

const int GedaElementGrammar:: rhs[] = {
  7, 11, 1, 2, 1, 1, 1, 1, 2, 1, 
  1, 3, 3, 9, 2, 1, 1, 3, 3, 10, 
  2, 1, 1, 3, 3, 5, 2, 3, 3, 3, 
  3, 7, 1, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 1, 2};


#ifndef QLALR_NO_GEDAELEMENTGRAMMAR_DEBUG_INFO
const int GedaElementGrammar::rule_info [] = {
    12, 13, 6, 14, 7, 6, 15, 7
  , 14, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26
  , 15, 27
  , 15, 27, 15
  , 27, 28
  , 27, 29
  , 27, 30
  , 27, 31
  , 28, 32, 33
  , 33, 34
  , 33, 35
  , 34, 6, 36, 7
  , 35, 8, 36, 9
  , 36, 37, 38, 39, 40, 41, 42, 43, 44, 45
  , 29, 46, 47
  , 47, 48
  , 47, 49
  , 48, 6, 50, 7
  , 49, 8, 50, 9
  , 50, 51, 52, 53, 54, 39, 40, 41, 43, 55, 45
  , 31, 56, 57
  , 57, 58
  , 57, 59
  , 58, 6, 60, 7
  , 59, 8, 60, 9
  , 60, 51, 52, 53, 54, 39
  , 30, 61, 62
  , 62, 6, 63, 7
  , 62, 8, 63, 9
  , 64, 6, 63, 7
  , 65, 8, 63, 9
  , 63, 37, 38, 66, 67, 68, 69, 39
  , 55, 70
  , 37, 71
  , 51, 71
  , 53, 71
  , 38, 71
  , 52, 71
  , 54, 71
  , 39, 71
  , 40, 71
  , 41, 71
  , 42, 71
  , 43, 70
  , 44, 70
  , 45, 71
  , 66, 71
  , 67, 71
  , 68, 71
  , 69, 71
  , 16, 71
  , 17, 70
  , 18, 70
  , 19, 70
  , 20, 71
  , 21, 71
  , 22, 71
  , 23, 71
  , 24, 71
  , 25, 71
  , 26, 71
  , 71, 10
  , 70, 11
  , 13, 1
  , 32, 3
  , 46, 2
  , 56, 4
  , 61, 5
  , 72, 12, 0};

const int GedaElementGrammar::rule_index [] = {
  0, 8, 20, 22, 25, 27, 29, 31, 33, 36, 
  38, 40, 44, 48, 58, 61, 63, 65, 69, 73, 
  84, 87, 89, 91, 95, 99, 105, 108, 112, 116, 
  120, 124, 132, 134, 136, 138, 140, 142, 144, 146, 
  148, 150, 152, 154, 156, 158, 160, 162, 164, 166, 
  168, 170, 172, 174, 176, 178, 180, 182, 184, 186, 
  188, 190, 192, 194, 196, 198, 200, 202, 204};
#endif // QLALR_NO_GEDAELEMENTGRAMMAR_DEBUG_INFO

const int GedaElementGrammar::action_default [] = {
  0, 64, 0, 0, 0, 62, 0, 0, 51, 0, 
  0, 68, 67, 66, 65, 0, 7, 0, 8, 0, 
  6, 0, 5, 3, 0, 0, 0, 27, 0, 34, 
  0, 29, 37, 0, 0, 47, 0, 48, 0, 49, 
  0, 50, 32, 40, 0, 28, 0, 0, 23, 22, 
  21, 0, 35, 0, 25, 38, 0, 36, 0, 39, 
  0, 26, 0, 24, 0, 0, 17, 16, 15, 0, 
  0, 19, 0, 0, 0, 0, 0, 41, 0, 42, 
  0, 63, 44, 0, 33, 20, 46, 0, 18, 0, 
  0, 11, 10, 9, 0, 0, 13, 0, 0, 0, 
  0, 0, 43, 0, 0, 45, 14, 0, 12, 4, 
  1, 0, 52, 0, 53, 54, 0, 0, 55, 0, 
  56, 57, 0, 58, 0, 59, 0, 60, 0, 61, 
  2, 69};

const int GedaElementGrammar::goto_default [] = {
  3, 2, 6, 24, 7, 111, 113, 116, 117, 119, 
  122, 124, 126, 128, 130, 23, 22, 20, 16, 18, 
  21, 93, 92, 91, 94, 30, 33, 42, 76, 78, 
  101, 80, 104, 85, 19, 68, 67, 66, 69, 53, 
  56, 58, 60, 83, 17, 50, 49, 48, 51, 15, 
  27, 28, 0, 0, 34, 36, 38, 40, 82, 52, 
  0};

const int GedaElementGrammar::action_index [] = {
  4, -12, -2, 11, -3, -12, -5, -11, -12, 2, 
  36, -12, -12, -12, -12, 23, -12, 26, -12, 20, 
  -12, 19, -12, 36, 7, 8, 12, -12, 14, -12, 
  5, -12, -12, 5, 5, -12, 5, -12, 5, -12, 
  5, -12, -12, -12, 3, -12, 8, 5, -12, -12, 
  -12, 10, -12, 5, -12, -12, 5, -12, 5, -12, 
  5, -12, 6, -12, 5, 5, -12, -12, -12, 0, 
  5, -12, 5, 5, 5, 8, 8, -12, -11, -12, 
  -11, -12, -12, 8, -12, -12, -12, 9, -12, 8, 
  8, -12, -12, -12, -6, -9, -12, -9, -9, -9, 
  -9, -11, -12, -11, -9, -12, -12, -1, -12, -12, 
  -12, -11, -12, -11, -12, -12, -3, -9, -12, -3, 
  -12, -12, -3, -12, -9, -12, -3, -12, -9, -12, 
  -12, -12, 

  -61, -61, -61, -61, -45, -61, -61, -41, -61, -61, 
  -61, -61, -61, -61, -61, -61, -61, -61, -61, -61, 
  -61, -61, -61, 37, -61, -35, -25, -61, -61, -61, 
  -28, -61, -61, -37, -39, -61, -38, -61, -56, -61, 
  -55, -61, -61, -61, -61, -61, -61, -42, -61, -61, 
  -61, -61, -61, -50, -61, -61, -49, -61, -57, -61, 
  10, -61, -61, -61, -21, 4, -61, -61, -61, -61, 
  -4, -61, -9, -7, 3, -32, -40, -61, -61, -61, 
  -29, -61, -61, -36, -61, -61, -61, -61, -61, 8, 
  22, -61, -61, -61, -61, 18, -61, 14, 11, 9, 
  -31, -30, -61, -33, 12, -61, -61, -61, -61, -61, 
  -61, -53, -61, -58, -61, -61, -46, -51, -61, -44, 
  -61, -61, -43, -61, -52, -61, -48, -61, -47, -61, 
  -61, -61};

const int GedaElementGrammar::action_info [] = {
  81, 5, 9, 96, 4, 1, 108, 5, 10, 71, 
  45, 131, 0, 63, 110, 5, 88, 0, 5, 54, 
  0, 0, 5, 31, 0, 90, 65, 89, 64, 26, 
  0, 25, 47, 0, 46, 0, 0, 0, 13, 14, 
  12, 11, 0, 0, 0, 0, 0, 0, 

  115, 103, 59, 41, 43, 114, 62, 125, 120, 55, 
  57, 127, 129, 118, 8, 121, 123, 112, 70, 79, 
  37, 39, 35, 86, 29, 105, 44, 77, 102, 84, 
  75, 32, 73, 95, 29, 74, 72, 61, 100, 99, 
  109, 98, 87, 70, 97, 106, 107, 95, 0, 0, 
  57, 0, 59, 0, 0, 55, 0, 0, 0, 0, 
  0, 0, 43, 0, 0, 0, 0, 29, 79, 43, 
  77, 86, 0, 43, 0, 0, 0, 32, 0, 0, 
  0, 29, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0};

const int GedaElementGrammar::action_check [] = {
  11, 10, 7, 9, 6, 1, 7, 10, 6, 9, 
  7, 0, -1, 7, 7, 10, 7, -1, 10, 9, 
  -1, -1, 10, 9, -1, 6, 6, 8, 8, 6, 
  -1, 8, 6, -1, 8, -1, -1, -1, 2, 3, 
  4, 5, -1, -1, -1, -1, -1, -1, 

  58, 31, 59, 59, 59, 58, 48, 59, 59, 59, 
  59, 59, 59, 59, 59, 59, 59, 58, 39, 59, 
  59, 59, 59, 59, 59, 58, 51, 59, 59, 58, 
  27, 59, 41, 25, 59, 42, 40, 27, 29, 28, 
  3, 27, 38, 39, 26, 33, 24, 25, -1, -1, 
  59, -1, 59, -1, -1, 59, -1, -1, -1, -1, 
  -1, -1, 59, -1, -1, -1, -1, 59, 59, 59, 
  59, 59, -1, 59, -1, -1, -1, 59, -1, -1, 
  -1, 59, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1};

