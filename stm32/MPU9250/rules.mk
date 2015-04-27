# Standard things
sp := $(sp).x
dirstack_$(sp) := $(d)
d := $(dir)
BUILDDIRS += $(BUILD_PATH)/$(d)
BUILDDIRS += $(BUILD_PATH)/$(d)/core/driver/eMPL
BUILDDIRS += $(BUILD_PATH)/$(d)/core/driver/stm32L
BUILDDIRS += $(BUILD_PATH)/$(d)/core/mllite
BUILDDIRS += $(BUILD_PATH)/$(d)/core/eMPL-hal

# Local flags
CFLAGS_$(d) := $(WIRISH_INCLUDES) $(LIBMAPLE_INCLUDES)

# Local rules and targets
cSRCS_$(d) := mpucontroller.c \
			  core/driver/eMPL/inv_mpu_dmp_motion_driver.c \
			  core/driver/eMPL/inv_mpu.c \
			  core/mllite/mpl.c \
			  core/mllite/storage_manager.c \
			  core/mllite/results_holder.c \
			  core/mllite/start_manager.c \
			  core/mllite/data_builder.c \
			  core/mllite/message_layer.c \
			  core/mllite/ml_math_func.c \
			  core/mllite/mlmath.c \
			  core/mllite/hal_outputs.c \
			  core/driver/stm32L/log_stm32l.c \
			  core/eMPL-hal/eMPL_outputs.c		  

cppSRCS_$(d) := core/driver/eMPL/stm32_shim.cpp

cFILES_$(d) := $(cSRCS_$(d):%=$(d)/%)
cppFILES_$(d) := $(cppSRCS_$(d):%=$(d)/%)

OBJS_$(d) := $(cFILES_$(d):%.c=$(BUILD_PATH)/%.o) \
                 $(cppFILES_$(d):%.cpp=$(BUILD_PATH)/%.o)
DEPS_$(d) := $(OBJS_$(d):%.o=%.d)

$(OBJS_$(d)): TGT_CFLAGS := $(CFLAGS_$(d))

TGT_BIN += $(OBJS_$(d))

# Standard things
-include $(DEPS_$(d))
d := $(dirstack_$(sp))
sp := $(basename $(sp))