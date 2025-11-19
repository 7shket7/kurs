CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -pedantic
LDFLAGS = -lssl -lcrypto

SOURCES = main.cpp client.cpp client_auth.cpp client_network.cpp
OBJECTS = $(SOURCES:.cpp=.o)
TARGET = client

TEST_SOURCES = simple_test.cpp
TEST_OBJECTS = $(TEST_SOURCES:.cpp=.o) $(filter-out main.o, $(OBJECTS))
TEST_TARGET = client_test

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

$(TEST_TARGET): $(TEST_OBJECTS)
	$(CXX) $(TEST_OBJECTS) -o $(TEST_TARGET) $(LDFLAGS)

%.o: %.cpp client.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: $(TEST_TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET) $(TEST_OBJECTS) $(TEST_TARGET) test_*.txt test_*.conf

help:
	@echo "Доступные команды:"
	@echo "  make all        - собрать основную программу"
	@echo "  make            - то же, что make all"
	@echo "  make test       - собрать и запустить тесты"
	@echo "  make test-build - только собрать тесты"
	@echo "  make clean      - очистить все сгенерированные файлы"
	@echo "  make help       - показать эту справку"

.PHONY: all clean test test-build help
