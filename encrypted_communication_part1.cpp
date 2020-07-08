/* --------------------------------------------------
*  Name: YongQuan Zhang, Shufen Situ
*  ID: 1515873, 1571076
*  CMPUT 274, Fall 2019
*  Assignment 2: Encrypted Arduino Communication part 1
*---------------------------------------------------*/

#include <Arduino.h>
int portpin;
void setup(){
	/* Set up function and set up the pin number and etc..
	Args:
		None.
	Returns:
		None.
	*/
	init();
	Serial.begin(9600);
	Serial3.begin(9600);
	portpin = 13;
	pinMode(portpin,INPUT);
}

void uint32_to_serial3(uint32_t num){
	/* A function that will write the encrypted message to serial 3.
	Args: 
		uint32_t num.
	Returns:
		None.
	*/
	Serial3.write((char)(num >> 0));
	Serial3.write((char)(num >> 8));
	Serial3.write((char)(num >> 16));
	Serial3.write((char)(num >> 24));
}
uint32_t uint32_from_serial3(){
	/* A function that will read the encrypted message from serial 3.
	Args:
		None.
	Returns:
		None.
	*/
	uint32_t num = 0;
	num = num | ((uint32_t) Serial3.read()) << 0;
	num = num | ((uint32_t) Serial3.read()) << 8;
	num = num | ((uint32_t) Serial3.read()) << 16;
	num = num | ((uint32_t) Serial3.read()) << 24;
	return num;

}

uint32_t mulmod(uint32_t x, uint32_t b, uint32_t m){
	/* A function that will do the multiplication without overflow.

	Args:
		uint32_t x
		uint32_t b
		uint32_t m
	Returns:
		Sum: final answer for the multiplication in distribution form.
	*/
	int i;
	uint32_t sum = 0;
	uint32_t item;
	// In this for loop we consider the algorithm of binary multiplication
	// then seperate it into different parts and mod m to prevent overflow.
	for(i = 31; i >= 0; i--){
		item = (b >> i)&1;
		item = (item * x) % m;
		sum = (sum << 1) % m;
		sum = (sum + item) % m;
	}
	return sum;

}

// computes the value x^pow mod m ("x to the power of pow" mod m)
uint32_t powmod(uint32_t x, uint32_t pow, uint32_t m) {
	/* A function that will do the powmod thing, and this function will only
	use 32 bytes. We used mulmod function to prevent overflow.
	Args:
		uint32_t x
		uint32_t pow
		uint32_t m
	Returns:
		ans
	*/

  	uint32_t ans = 1;
  	uint32_t pow_x = x;

  	// NOTE: in the full assignment you will have to
  	// replace the 64-bit types with an algorithm that
  	// performs multiplication modulo a 31-bit number while
  	// only using 32-bit types.

  	while (pow > 0) {
    	if (pow&1 == 1) {
      	// Replace the following line with mulmod function.
      	ans = mulmod(ans,pow_x,m);
    	}

    	pow_x = mulmod(pow_x,pow_x,m);

    	// Divideds by 2.
    	pow >>= 1;
  	}

  	return ans;
}


uint32_t Encryption(uint32_t we_read, uint32_t e, uint32_t m){
	/* A function that will encryte your original message based on
	the public key, private key and modulus. We used powmod function
	to do that instead of using exponential function.
	Args:
		uint32_t we_read
		uint32_t e
		uint32_t m
	Returns:
		enc
	*/

	uint32_t enc;
	enc = powmod(we_read,e,m);
	return enc;

}
uint32_t Decryption(uint32_t num, uint32_t d, uint32_t n){
	/* A function that will decryte your original message based on
	the public key, private key and modulus. We used powmod function
	to do that instead of using exponential function.
	Args:
		uint32_t num
		uint32_t d
		uint32_t n
	Returns:
		dec
	*/

	uint32_t dec;
	dec = powmod(num,d,n);
	return dec;

}
int main(){
	/*Any code indented under this line will only be run
    when the program is called directly from the terminal
    using "make","make upload" and "serial-mon".

    Args:
     	None
     Return:
     	None
	*/
	setup();
	bool server_client;
	uint32_t e;
	uint32_t n;
	uint32_t d;
	uint32_t m;
	// Determine if the arduino mega board act as a server or client.
	if (digitalRead(portpin) == HIGH){
		server_client = true;
		Serial.println("I'm server");
		e = 11;
		d = 27103;
		n = 95477;
		m = 84823;
	}
	else{
		server_client = false;
		Serial.println("I'm client");
		e = 7;
		d = 38291;
		n = 84823;
		m = 95477;
	}
	while(true){
		if (Serial.available() > 0){
			uint32_t we_read = Serial.read();
			// When we type return key, it should print a new line
			// char.
			if (we_read == 13){
				Serial.print("\n");
			}
			uint32_t num = Encryption(we_read, e, m);
			uint32_to_serial3(num);
			// We wrte the thing to server's serial1 so we don't need
			// to turn on the local echo.
			Serial.write(we_read);
		}
		if (Serial3.available() > 3){
			uint32_t num = uint32_from_serial3();
			if (Decryption(num, d, n) == 13){
				Serial.print("\n");
			}
			Serial.write(Decryption(num, d, n));
		}
	}
	Serial.flush();
	return 0;
}
