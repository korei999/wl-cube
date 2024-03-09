SAFE_STACK :=
SAFE_STACK += -mshstk -fstack-protector-all 

ASAN :=
ASAN += -fsanitize=address

LOGS := -DLOGS

DEBUG :=
DEBUG += -DDEBUG 
DEBUG += $(LOGS)

WNO := 
WNO += -Wno-unused-parameter
WNO += -Wno-unused-variable
