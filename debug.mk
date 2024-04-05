SAFE_STACK :=
SAFE_STACK += -mshstk -fstack-protector-all 

ASAN :=
ASAN += -fno-omit-frame-pointer -fsanitize=address #-fsanitize=undefined


DEBUG :=
DEBUG += -DDEBUG 
DEBUG += -DLOGS
DEBUG += -DFPS_COUNTER
# DEBUG += -DMODEL

WNO := 
WNO += -Wno-unused-parameter
WNO += -Wno-unused-variable
