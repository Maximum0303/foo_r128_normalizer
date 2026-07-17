#pragma once

#define IDD_R128_CONFIG            101
#define IDD_R128_GLOSSARY          102
#define IDD_R128_TEXT_INFO         103
#define IDD_R128_CONFIRM_DEFAULTS  104

#define IDC_TARGET_LUFS            1001
#define IDC_MAX_BOOST              1002
#define IDC_MAX_ATTENUATION        1003
#define IDC_TRUE_PEAK              1004
#define IDC_RESET_EACH_TRACK       1005
#define IDC_ENABLE_PEAK_GUARD      1006
#define IDC_STATUS_TEXT            1007
#define IDC_DEFAULTS               1008

#define IDC_DIAG_INTEGRATED        1009
#define IDC_DIAG_GAIN              1010
#define IDC_DIAG_TRUE_PEAK         1011
#define IDC_DIAG_PEAK_GUARD        1012
#define IDC_DIAG_CHANNEL_LAYOUT    1013
#define IDC_LOOKAHEAD_MS           1014
#define IDC_LIMITER_RELEASE_MS     1015
#define IDC_DIAG_LIMITER_REDUCTION 1016
#define IDC_DIAG_LATENCY           1017

#define IDC_DIAG_MOMENTARY        1018
#define IDC_DIAG_SHORT_TERM       1019
#define IDC_DIAG_LRA              1020

#define IDC_STARTUP_ANALYSIS_SECONDS 1021
#define IDC_SILENCE_GUARD_LUFS      1022
#define IDC_ENABLE_SILENCE_GUARD    1023
#define IDC_DIAG_NORMALIZATION_STATE 1024

#define IDC_GAIN_LOCK_SECONDS       1025
#define IDC_GAIN_LOCK_TOLERANCE_LU  1026
#define IDC_ENABLE_GAIN_LOCK        1027
#define IDC_DIAG_GAIN_LOCK          1028

#define IDC_PROFILE_STANDARD       1029
#define IDC_PROFILE_STREAMING      1030
#define IDC_PROFILE_BROADCAST      1031
#define IDC_PROFILE_NIGHT          1032
#define IDC_RESET_MEASUREMENT      1033
#define IDC_COPY_DIAGNOSTICS       1034

#define IDC_PROFILE_DESCRIPTION    1035

#define IDC_APPLY_SETTINGS         1036
#define IDC_APPLY_STATUS           1037

#define IDC_SHOW_LICENSE           1038

#define IDC_ENABLE_MODERN_BOOST       1039
#define IDC_MODERN_STRENGTH           1040
#define IDC_PROFILE_MODERN            1041
#define IDC_DIAG_COMPRESSOR_REDUCTION 1042
#define IDC_DIAG_CLIPPER_REDUCTION    1043

#define IDC_DIAG_OUTPUT_INTEGRATED    1044
#define IDC_DIAG_TARGET_DIFFERENCE     1045
#define IDC_DIAG_PROCESSING_RISK       1046
#define IDC_DIAG_SAFETY_REDUCTION      1047
#define IDC_ORIGINAL_COMPARE           1048
#define IDC_ENABLE_ADAPTIVE_MASTER 1049
#define IDC_PROFILE_ADAPTIVE       1050
#define IDC_DIAG_MAX_TRUE_PEAK              1051
#define IDC_DIAG_MAX_COMPRESSOR_REDUCTION   1052
#define IDC_DIAG_MAX_CLIPPER_REDUCTION      1053
#define IDC_DIAG_MAX_LIMITER_REDUCTION      1054
#define IDC_DIAG_CLIP_EVENT_COUNT           1055
#define IDC_DIAG_PROCESSING_EVALUATION      1056
#define IDC_DIAG_SAMPLE_RATE                1057
#define IDC_DIAG_CPU_LOAD                   1058
#define IDC_SHOW_DIAGNOSTIC_HELP        1059
#define IDC_ENABLE_THREE_BAND_MASTER   1060
#define IDC_PROFILE_THREE_BAND         1061
#define IDC_DIAG_THREE_BAND_REDUCTION  1062
#define IDC_COMPARE_LOUDNESS_MATCH      1063
#define IDC_GLOSSARY_LIST             1064
#define IDC_GLOSSARY_DESCRIPTION      1065
#define IDC_CONFIG_TABS                    1066
#define IDC_TEXT_INFO_BODY                 1067

#define IDC_BASIC_HEADER                   1100
#define IDC_LABEL_TARGET_LUFS              1101
#define IDC_UNIT_TARGET_LUFS               1102
#define IDC_LABEL_MAX_BOOST                1103
#define IDC_UNIT_MAX_BOOST                 1104
#define IDC_LABEL_MAX_ATTENUATION          1105
#define IDC_UNIT_MAX_ATTENUATION           1106
#define IDC_LABEL_TRUE_PEAK                1107
#define IDC_UNIT_TRUE_PEAK                 1108
#define IDC_LABEL_LOOKAHEAD                1109
#define IDC_UNIT_LOOKAHEAD                 1110
#define IDC_LABEL_LIMITER_RELEASE          1111
#define IDC_UNIT_LIMITER_RELEASE           1112
#define IDC_LABEL_STARTUP                  1113
#define IDC_UNIT_STARTUP                   1114
#define IDC_LABEL_SILENCE_THRESHOLD        1115
#define IDC_UNIT_SILENCE_THRESHOLD         1116
#define IDC_LABEL_GAIN_LOCK_SECONDS        1117
#define IDC_UNIT_GAIN_LOCK_SECONDS         1118
#define IDC_LABEL_GAIN_LOCK_TOLERANCE      1119
#define IDC_UNIT_GAIN_LOCK_TOLERANCE       1120
#define IDC_BASIC_RIGHT_HEADER             1121

#define IDC_ADV_HEADER                     1130
#define IDC_LABEL_MODERN_STRENGTH          1131
#define IDC_UNIT_MODERN_STRENGTH           1132
#define IDC_ADV_INFO_1                     1133
#define IDC_ADV_INFO_2                     1134
#define IDC_ADV_INFO_3                     1135
#define IDC_ADV_GROUP_MODERN               1136
#define IDC_ADV_GROUP_ADAPTIVE             1137
#define IDC_ADV_GROUP_THREE_BAND           1138
#define IDC_ADV_GROUP_STRENGTH              1139

#define IDC_LABEL_DIAG_NORMALIZATION       1150
#define IDC_LABEL_DIAG_MOMENTARY           1151
#define IDC_LABEL_DIAG_SHORT_TERM          1152
#define IDC_LABEL_DIAG_GAIN_LOCK           1153
#define IDC_LABEL_DIAG_INPUT_INT           1154
#define IDC_LABEL_DIAG_OUTPUT_INT          1155
#define IDC_LABEL_DIAG_TARGET_DIFF         1156
#define IDC_LABEL_DIAG_LRA                 1157
#define IDC_LABEL_DIAG_GAIN                1158
#define IDC_LABEL_DIAG_PROCESSING          1159
#define IDC_LABEL_DIAG_SAFETY              1160
#define IDC_LABEL_DIAG_COMPRESSOR          1161
#define IDC_LABEL_DIAG_CLIPPER             1162
#define IDC_LABEL_DIAG_LIMITER             1163
#define IDC_LABEL_DIAG_TRUE_PEAK           1164
#define IDC_LABEL_DIAG_LATENCY             1165
#define IDC_LABEL_DIAG_PEAK_GUARD          1166
#define IDC_LABEL_DIAG_SAMPLE_RATE         1167
#define IDC_LABEL_DIAG_CHANNEL_LAYOUT      1168
#define IDC_LABEL_DIAG_CPU                 1169
#define IDC_LABEL_DIAG_CLIP_EVENTS         1170
#define IDC_LABEL_DIAG_THREE_BAND          1171
#define IDC_LABEL_DIAG_EVALUATION          1172
#define IDC_LABEL_DIAG_MAX_TRUE_PEAK       1173
#define IDC_LABEL_DIAG_MAX_COMPRESSOR      1174
#define IDC_LABEL_DIAG_MAX_CLIPPER         1175
#define IDC_LABEL_DIAG_MAX_LIMITER         1176
#define IDC_DIAG_LEFT_HEADER               1177
#define IDC_DIAG_RIGHT_HEADER              1178
