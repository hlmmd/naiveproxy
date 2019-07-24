#搜索当前目录下的所有.cpp文件，作为libqevent的源文件
aux_source_directory(. SRC_LIST) 
#add_library(nproxy  SHARED ${SRC_LIST}) # 生成动态链接库
#add_library(nproxy  ${SRC_LIST})

add_executable(naiveproxy ${SRC_LIST})


# 安装 libqevent 到lib中
#install (TARGETS qevent DESTINATION lib)
