################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
FreeRTOS_source/portable/MemMang/heap_1.obj: E:/Predavanja/semestarVIII/SRV/MSP430_FreeRTOS-master/Examples/common/FreeRTOS_source/portable/MemMang/heap_1.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"E:/Predavanja/semestar VI/MRS/ccs/tools/compiler/ti-cgt-msp430_21.6.1.LTS/bin/cl430" -vmspx --data_model=restricted --use_hw_mpy=F5 --include_path="E:/Predavanja/semestar VI/MRS/ccs/ccs_base/msp430/include" --include_path="E:/Predavanja/semestarVIII/SRV/MSP430_FreeRTOS-master/Examples/SRV_bonus" --include_path="E:/Predavanja/semestarVIII/SRV/MSP430_FreeRTOS-master/Examples/common/drivers" --include_path="E:/Predavanja/semestarVIII/SRV/MSP430_FreeRTOS-master/Examples/common/FreeRTOS_source/include" --include_path="E:/Predavanja/semestarVIII/SRV/MSP430_FreeRTOS-master/Examples/common/FreeRTOS_source/portable/CCS/MSP430X" --include_path="E:/Predavanja/semestarVIII/SRV/MSP430_FreeRTOS-master/Examples/SRV_bonus" --include_path="E:/Predavanja/semestarVIII/SRV/MSP430_FreeRTOS-master/Examples/common/drivers/MSP430F5xx_6xx" --include_path="E:/Predavanja/semestar VI/MRS/ccs/tools/compiler/ti-cgt-msp430_21.6.1.LTS/include" --advice:power=all --define=__MSP430F5529__ -g --printf_support=minimal --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --preproc_with_compile --preproc_dependency="FreeRTOS_source/portable/MemMang/$(basename $(<F)).d_raw" --obj_directory="FreeRTOS_source/portable/MemMang" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


