PACKAGES       =  test stmlib/utils stmlib/dsp

VPATH          = $(PACKAGES)

TARGET         = tapo_test
BUILD_ROOT     = build/
BUILD_DIR      = $(BUILD_ROOT)$(TARGET)/
CC_FILES       = tapo_test.cc \
		multitap_delay.cc \
		tap_allocator.cc \
		random.cc \
		resources.cc

OBJ_FILES      = $(CC_FILES:.cc=.o)
OBJS           = $(patsubst %,$(BUILD_DIR)%,$(OBJ_FILES)) $(STARTUP_OBJ)
DEPS           = $(OBJS:.o=.d)
DEP_FILE       = $(BUILD_DIR)depends.mk
SAMPLE_RATE    = 48000

all:  tapo_test

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)%.o: %.cc
	g++ -c -DTEST -g -Wall -Werror -I. \
	-DSAMPLE_RATE=$(SAMPLE_RATE) \
	-fno-exceptions -fno-rtti \
	$< -o $@

$(BUILD_DIR)%.d: %.cc
	g++ -MM -DTEST -I. $< -MF $@ -MT $(@:.d=.o)

tapo_test:  $(OBJS)
	g++ -o $(TARGET) $(OBJS)

depends:  $(DEPS)
	cat $(DEPS) > $(DEP_FILE)

$(DEP_FILE):  $(BUILD_DIR) $(DEPS)
	cat $(DEPS) > $(DEP_FILE)

include $(DEP_FILE)
