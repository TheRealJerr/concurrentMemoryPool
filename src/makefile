# 编译器和标志
CXX = g++
CXXFLAGS = -std=c++17 -Wall  -g 
LDFLAGS = -lpthread

# 文件列表
SRC = $(wildcard *.cpp)
OBJ = $(SRC:.cpp=.o)
BIN = bin

$(BIN): $(OBJ)
	$(CXX) $(OBJ) $(LDFLAGS) -o $@

# 编译规则
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 清理
.PHONY: clean
clean:
	rm -f $(BIN) $(OBJ)
