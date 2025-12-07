CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -I.
LDFLAGS = -lssl -lcrypto

# Основная программа
SRCS = main.cpp \
    Client.cpp \
    ErrorHandler.cpp \
    Authenticator.cpp \
    DataProcessor.cpp \
    ServerConnection.cpp

OBJS = $(SRCS:.cpp=.o)
TARGET = client

# UnitTest
TEST_CXXFLAGS = $(CXXFLAGS:-Werror=) -I/usr/local/include
TEST_LDFLAGS = $(LDFLAGS) -L/usr/local/lib -lUnitTest++
TEST_SRCS = ClientUnitTest.cpp
TEST_OBJS = $(TEST_SRCS:.cpp=.o)
TEST_TARGET = client_tests

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TEST_TARGET): $(TEST_OBJS)
	$(CXX) $(TEST_OBJS) -o $(TEST_TARGET) $(TEST_LDFLAGS)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -f $(OBJS) $(TARGET) $(TEST_OBJS) $(TEST_TARGET)

install:
	cp $(TARGET) /usr/local/bin/

uninstall:
	rm -f /usr/local/bin/$(TARGET)

.PHONY: all clean install uninstall test
