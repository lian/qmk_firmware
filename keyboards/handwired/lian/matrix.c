#include QMK_KEYBOARD_H
#include "i2c_master.h"

extern i2c_status_t mcp23017_status;
#define I2C_TIMEOUT 1000

#define MCP23017_A0 0
#define MCP23017_A1 0
#define MCP23017_A2 0
#define I2C_ADDR ((0x20|(MCP23017_A0<<0)|(MCP23017_A1<<1)|(MCP23017_A2<<2)) << 1)
#define I2C_ADDR_WRITE (I2C_ADDR | I2C_WRITE)
#define I2C_ADDR_READ (I2C_ADDR | I2C_READ)

// Register addresses
// See https://github.com/adafruit/Adafruit-MCP23017-Arduino-Library/blob/master/Adafruit_MCP23017.h
#define IODIRA 0x00  // i/o direction register
#define IODIRB 0x01
#define IPOLA 0x02 // input polarity register
#define IPOLB 0x03
#define GPPUA 0x0C  // GPIO pull-up resistor register
#define GPPUB 0x0D
#define GPIOA 0x12  // general purpose i/o port register (write modifies OLAT)
#define GPIOB 0x13

bool         i2c_initialized = 0;
i2c_status_t mcp23017_status = I2C_ADDR;

uint8_t init_mcp23017(void) {
    print("starting init_mcp23017\n");
    mcp23017_status = I2C_ADDR;

    // I2C subsystem
    if (i2c_initialized == 0) {
        i2c_init();  // on pins D(1,0)
        i2c_initialized = true;
        wait_ms(I2C_TIMEOUT);
    }

    // set pin direction
    // enable input for A0-A7 + B0-B3
    // enable output for B4-B7
    mcp23017_status = i2c_start(I2C_ADDR_WRITE, I2C_TIMEOUT);
    if (mcp23017_status) goto out;
    mcp23017_status = i2c_write(IODIRA, I2C_TIMEOUT);
    if (mcp23017_status) goto out;
    mcp23017_status = i2c_write(0b11111111, I2C_TIMEOUT);
    if (mcp23017_status) goto out;
    mcp23017_status = i2c_write(0b00001111, I2C_TIMEOUT);
    if (mcp23017_status) goto out;
    i2c_stop();

    // set pull-up
    // enable pull-up for A0-A7 + B0-B3
    mcp23017_status = i2c_start(I2C_ADDR_WRITE, I2C_TIMEOUT);
    if (mcp23017_status) goto out;
    mcp23017_status = i2c_write(GPPUA, I2C_TIMEOUT);
    if (mcp23017_status) goto out;
    mcp23017_status = i2c_write(0b11111111, I2C_TIMEOUT);
    if (mcp23017_status) goto out;
    mcp23017_status = i2c_write(0b00001111, I2C_TIMEOUT);
    if (mcp23017_status) goto out;

    // set input polarity
    // enable pull-up for A0-A7 + B0-B3
    // 1 = GPIO register bit reflects the opposite logic state of the input pin
    mcp23017_status = i2c_start(I2C_ADDR_WRITE, I2C_TIMEOUT);
    if (mcp23017_status) goto out;
    mcp23017_status = i2c_write(IPOLA, I2C_TIMEOUT);
    if (mcp23017_status) goto out;
    mcp23017_status = i2c_write(0b11111111, I2C_TIMEOUT);
    if (mcp23017_status) goto out;
    mcp23017_status = i2c_write(0b00001111, I2C_TIMEOUT);
    if (mcp23017_status) goto out;

out:
    i2c_stop();
    return mcp23017_status;
}

/* matrix state(1:on, 0:off) */
static matrix_row_t matrix[MATRIX_ROWS];      // debounced values
static matrix_row_t read_cols(uint8_t row);
static void         select_row(uint8_t row);
static uint8_t mcp23017_reset_loop;

void matrix_init_custom(void) {
    mcp23017_status = init_mcp23017();

    for (uint8_t i = 0; i < MATRIX_ROWS; i++) {
        matrix[i]     = 0;
    }
}

void matrix_power_up(void) {
    matrix_init_custom();
}

// Reads and stores a row, returning
// whether a change occurred.
static inline bool store_matrix_row(matrix_row_t current_matrix[], uint8_t index) {
    matrix_row_t temp = read_cols(index);
    if (current_matrix[index] != temp) {
        current_matrix[index] = temp;
        return true;
    }
    return false;
}

bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    if (mcp23017_status) {  // if there was an error
        if (++mcp23017_reset_loop == 0) {
            // since mcp23017_reset_loop is 8 bit - we'll try to reset once in 255 matrix scans
            // this will be approx bit more frequent than once per second
            dprint("trying to reset mcp23017\n");
            mcp23017_status = init_mcp23017();
            if (mcp23017_status) {
                dprint("mcp23017 not responding\n");
            } else {
                dprint("mcp23017 attached\n");
            }
        }
    }

    bool changed = false;
    for (uint8_t i = 0; i < MATRIX_ROWS; i++) {
        select_row(i);
        changed |= store_matrix_row(current_matrix, i);
    }

    return changed;
}

static matrix_row_t read_cols(uint8_t row) {
    if (mcp23017_status) {  // if there was an error
        return 0;
    }

    uint16_t data    = 0;

    mcp23017_status = i2c_start(I2C_ADDR_WRITE, I2C_TIMEOUT);
    if (mcp23017_status) goto out;
    mcp23017_status = i2c_write(GPIOA, I2C_TIMEOUT);
    if (mcp23017_status) goto out;
    mcp23017_status = i2c_start(I2C_ADDR_READ, I2C_TIMEOUT);
    if (mcp23017_status) goto out;
    mcp23017_status = i2c_read_nack(I2C_TIMEOUT);
    if (mcp23017_status < 0) goto out;

    // flip bits of mcp23017 output (0 pressed) to matrix_row_t (1 pressed)
    //data = ~((uint8_t)mcp23017_status);
    data = (uint8_t)mcp23017_status;

    if (MATRIX_COLS > 8) {
      mcp23017_status = i2c_start(I2C_ADDR_WRITE, I2C_TIMEOUT);
      if (mcp23017_status) goto out;
      mcp23017_status = i2c_write(GPIOB, I2C_TIMEOUT);
      if (mcp23017_status) goto out;
      mcp23017_status = i2c_start(I2C_ADDR_READ, I2C_TIMEOUT);
      if (mcp23017_status) goto out;
      mcp23017_status = i2c_read_nack(I2C_TIMEOUT);
      if (mcp23017_status < 0) goto out;

      // flip bits of mcp23017 output (0 pressed) to matrix_row_t (1 pressed)
      // uint16 = GPIOB(00001111) + GPIOA(11111111)
      //data |= (~((uint8_t)mcp23017_status) << 8);
      data |= ((uint8_t)mcp23017_status << 8);
    }

    mcp23017_status = I2C_STATUS_SUCCESS;

out:
    i2c_stop();
    return data;
}

static void select_row(uint8_t row) {
  if (mcp23017_status) {  // if there was an error
    return;
  }

  mcp23017_status = i2c_start(I2C_ADDR_WRITE, I2C_TIMEOUT);
  if (mcp23017_status) goto out;
  mcp23017_status = i2c_write(GPIOB, I2C_TIMEOUT);
  if (mcp23017_status) goto out;


  // select one of { B4, B5, B6, B7 } as output (0 value) and the rest to input (1 value)
  mcp23017_status = i2c_write(0xFF & ~(1 << (row + (8 - MATRIX_ROWS))), I2C_TIMEOUT);

  if (mcp23017_status) goto out;
out:
  i2c_stop();
}
