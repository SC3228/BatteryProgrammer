// EEPROM include file

uint8_t EEPROM_Read(uint8_t *buffer = NULL);
void EEPROM_Write(uint8_t *buffer = NULL);
void EEPROM_Update(uint8_t type);
void EEPROM_Verify(uint8_t *buffer = NULL);
