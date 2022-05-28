
include config.mk

TARGET := $(BIN_DIR)/$(BIN)

OBJ := $(patsubst $(SRC_DIR)%, $(OBJ_DIR)%, $(SRC:.c=.o))


$(TARGET): $(OBJ)
	@mkdir -p $(BIN_DIR)
	@$(CC) $(LDFLAGS) $^ -o $@


$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@


all: $(TARGET)


clean:
	@echo cleaning...
	@$(RM) -rv $(BIN_DIR) $(OBJ_DIR)


rebuild:
	make clean && make


install: all
	@mkdir -p $(INSTALL_PREFIX)/bin
	@cp -f $(TARGET) $(INSTALL_PREFIX)/bin
	@chmod 755 $(INSTALL_PREFIX)/bin/$(BIN)


uninstall:
	@rm -rf $(INSTALL_PREFIX)/bin/$(BIN)


.PHONY: all clean rebuild install uninstall
