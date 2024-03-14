SAFE_STACK :=
SAFE_STACK += -mshstk -fstack-protector-all 

ASAN :=
ASAN += -fsanitize=address

LOGS := -DLOGS
MODEL := -DMODEL

DEBUG :=
DEBUG += -DDEBUG 
DEBUG += $(LOGS)
DEBUG += $(MODEL)

WNO := 
WNO += -Wno-unused-parameter
WNO += -Wno-unused-variable
