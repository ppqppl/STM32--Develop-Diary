CXX = g++
LIBS = `pkg-config --libs opencv`
CFLAGS = `pkg-config --cflags opencv`
CPPCFLAGS = -g -std = c++11 -Werror

SRCS = test1.cpp
TARGET = test1
OBJS = test1.o

# 编译生成可执行文件
$(TARGET): $(OBJS)
	@echo "Start Compiling……"
	@echo $(CXX)
	$(CXX) $(INC) $(CPPFLAGS) $(OBJS) -o $(TARGET) $(LIBS)
	@echo "Compile Done!"

# 下面是顺序编译生成 .o 文件
# 每个 .c/.cpp 文件都会编译生成对应的 .o 文件
# < 表示目标依赖表中的第一个依赖文件; @ 表示生成的对应的目标文件
$(OBJS):%.o:%.cpp
	$(CXX) $(INC) $(CPPFLAGS) -c $< -o $@ 

.PHONY : clean
clean :
	-rm -r *.o $(TARGET)
