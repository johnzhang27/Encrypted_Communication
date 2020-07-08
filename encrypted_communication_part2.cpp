/* --------------------------------------------------
*  Name: YongQuan Zhang, Shufen Situ
*  ID: 1515873, 1571076
*  CMPUT 274, Fall 2019
*  Assignment 2: Encrypted Arduino Communication part 2
*---------------------------------------------------*/

#include <Arduino.h>
using namespace std;
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
unsigned int upper_sqrt(unsigned int n) {
    /* A function could be used to calculate the upper square root
        of a given number. This function is given by instructor.

        Args:
            unsigned int n

        Returns:
            d
    */
    unsigned int d = sqrt((double) n);
    while (d <= (1 << 16) && d*d <= n) {
        ++d;
    }

    return d;
}
void uint32_to_serial3(uint32_t num){
	/* A function that will write the encrypted message to serial 3.
		Provided by instructor.
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
		Provided by instructor.
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
	uint32_t term;
	// In this for loop we consider the algorithm of binary multiplication
	// then seperate it into different parts and mod m to prevent overflow.
	for(i = 31; i >= 0; i--){
		term = (b >> i)&1;
		term = (term * x) % m;
		sum = (sum << 1) % m;
		sum = (sum + term) % m;
	}
	return sum;

}
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

  	uint32_t ans = 1 % m;
  	uint32_t pow_x = x % m;

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
uint32_t Decryption(uint32_t num1, uint32_t d, uint32_t n){
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
	dec = powmod(num1,d,n);
	return dec;

}

uint32_t gcd_euclid_fast(uint32_t a, uint32_t b) {
    /* A function could be used to calculate the greatest common divisor.
        In my code, this function is used as a condition to find out "k" value
        where "k" is "b" when gcd(a,b) = 1.
        This function is provided by instructor.

        Args:
            uint32_t a
            uint32_t b

        Returns:
            a
    */
	while (b > 0) {
		a %= b;
	    uint32_t tmp = a;
	    a = b;
	    b = tmp;
	}
	return a;
}

bool randombit(){
	/* A function could generate true random least significant bit by
		determining the voltage flow on pin A1.
		Args:
			None
		Returns:
			1 or 0(true or false), if the significant bit is 1 then
			return "true", otherwise return "false".
	*/
	int val = analogRead(A1);
	delay(5);
	return ((val&1) == 1);
}

uint16_t generate_k(int k){
	/* A function could generate true random number by determining the
	voltage flow on pin A1(by using the randombit function).
	Args:
		int k: k is the number of bit that you want.
	Returns:
		a k bits random number which falls between 2^k and 2^k+1.
	*/
	uint32_t num = 0;
	for(int i = 0; i < k;i++){
		if(randombit()){
			num = num | (1ul << i);
		}
	}
	return (num | (1ul << k));
}

bool primetest(uint32_t k){
	/* A function could test for prime number, this function is
		my solution for one of the morning problem.
	Args:
		k: a random prime number.
	Returns:
		ans2: true or false.
	*/
	uint32_t ans;
    bool ans2 = false;
    ans = upper_sqrt(k);
    for(int i = 2; i <=(ans); i ++){
        if(k%i == 0 || k == 1){
            return (ans2 = true);
        }
    }
    // When return a true than means the primetest failed.
    return ans2;
}

uint32_t generate_e(){
	/* A function coule generate true random number by determining the
	voltage flow on pin A1.
	Args:
		None
	Returns:
		e: a random number that fall between 1 and 2^15.
	*/
	uint32_t e = 0;
	for(int i = 0; i < 15;i++){
		if(randombit()){
			e |= (1ul << i);
		}
	}
	return e;
}

uint32_t generate_d(uint32_t e, uint32_t phi_num){
	/* A function could generate d by using the Euclid's Extended Algorithm
		I combined the reduce_mod function with the generated_d function,
	Args:
		e: a valid e number that generate by generate_e function.
		phi_num: (p-1)*(q-1)
	Returns:
		d: a positive, valid d number that could be tested by using
		e*d % m = 1
	*/

	// Euclid's Extended Algorithm that copied from lecture slides.
	int32_t r[40];
	int32_t s[40];
	int32_t t[40];
	r[0] = e;
	r[1] = phi_num;
	s[0] = 1; s[1] = 0;
	t[0] = 0; t[1] = 1;
	int32_t i = 1;
	while (r[i] > 0){
		int32_t q = r[i-1]/r[i];
		r[i+1] = r[i-1] - q*r[i];
		s[i+1] = s[i-1] - q*s[i];
		t[i+1] = t[i-1] - q*t[i];
		i = i + 1;
	}
	int32_t d = s[i-1];
	return d;

}
uint32_t reduce_mod(int32_t d, uint32_t phi_num){
	/* This is the reduce_mod function part. It deals with the
	 	situation that when d is less than 0 or greater than phi_num.
	 	Args: d, could be negative.
	 		phi_num
	 	Returns:
	 		d, a positive d.
	 	*/
	if(d < 0){
		int32_t z = ((-d)/phi_num) + 1;
		d = (d+z*phi_num) % phi_num;
	}
	else if(d >= phi_num){
		d = d % phi_num;
	}
	return d;
}

bool  wait_on_serial3( uint8_t  nbytes , long  timeout ) {
	/* A function could be used for the timeout transition in
		handshaking process.
	Args:
		nbytes: number of bytes that you want to receive.
		timeout: how many ms you want to wait after you receive some bytes.
	Returns:
		true or false: if greater than or equal to nbytes, return true,
		 otherwise return false.
	*/
	unsigned  long  deadline = millis () + timeout;
	while (Serial3.available ()<nbytes  && (timeout <0 ||  millis ()<deadline)){
		delay (1);
	}
	return  Serial3.available () >=nbytes;
}

enum ClientState {
	/*A function that create states for client*/
   start, WFA, DataExchange
 };
enum ServerState {
	/*A function that create states for server
	WFK_1 is the state to cosume all extra CR.*/
  	listen, WFA_S, WFK, WFK_1, DataExchange_S
 };

bool client_action(uint8_t CR, uint8_t ACK,uint32_t ckey, uint32_t cmod,uint32_t &skey, uint32_t &smod ){
	/* A function will be used for client during handshaking process.
	Args:
		CR: "C"
		ACK: "A"
		ckey: client public key
		cmod: client modulus
		skey: server public key
		smod: server modulus
	Returns:
		true or false: if entered the dataexchange state then true,
		otherwise false.
		skey and smod: we used pass by reference to store those values.
	*/
	Serial.println("Start handshaking...");
  	ClientState state = start;
  	bool ans = false;
	while(true){
		if(state == start){
			Serial3.write(CR);
			uint32_to_serial3(ckey);
			uint32_to_serial3(cmod);
			state = WFA;
		}
		if(state == WFA){
			if(wait_on_serial3(9,1000)== false){
				state = start;
			}
			else{
				if(Serial3.read() == ACK){
					skey = uint32_from_serial3();
					smod = uint32_from_serial3();
					Serial3.write(ACK);
					state = DataExchange;
				}
			}
		}
		if(state == DataExchange){
			ans = true;
			Serial.println("Now you could start dataexchange.");
			return ans;
		}
	}
}

bool server_action(uint8_t CR, uint8_t ACK, uint32_t skey, uint32_t smod, uint32_t &ckey, uint32_t &cmod) {
	/* A function will be used for server during handshaking process.
	Args:
		CR: "C"
		ACK: "A"
		ckey: client public key
		cmod: client modulus
		skey: server public key
		smod: server modulus
	Returns:
		true or false: if entered the dataexchange state then true,
		otherwise false.
		ckey and cmod: we used pass by reference to store those values.
	*/
	Serial.println("Start handshaking...");
  	ServerState state = listen;
  	uint8_t read_1;
  	bool ans = false;
	while(true){
		if(state == listen){
			if(Serial3.available() > 8){
				if(Serial3.read() == CR){
					state = WFK;
				}
			}
		}
		if(state == WFK){
			if (wait_on_serial3(8,1000)== false){
				state = listen;
			}
			else{
				ckey = uint32_from_serial3();
				cmod = uint32_from_serial3();
				Serial3.write(ACK);
				uint32_to_serial3(skey);
				uint32_to_serial3(smod);
				state = WFA_S;
			}
		}
		if(state == WFA_S){
			if (wait_on_serial3(1,1000)== false){
				state = listen;
			}
			else{
					read_1 = Serial3.read();
					if (read_1 == CR){
						state = WFK_1;
					}
					else if(read_1 == ACK){
						state = DataExchange_S;
					}
			}

		}
		if(state == WFK_1){
			if (wait_on_serial3(8,1000) == false){
				state = listen;
			}
			else{
				state = WFA_S;
				ckey = uint32_from_serial3();
				cmod = uint32_from_serial3();
			}

		}
		if(state == DataExchange_S){
			Serial.println("Now you could start dataexchange.");
			ans = true;
			return ans;
		}
	}
}

void Generate_keys_for_server_and_client(uint32_t &p, uint32_t &q, uint32_t &n, uint32_t &phi_num, uint32_t &local_e, uint32_t &d){
	Serial.println("Generating keys...");
	p = generate_k(14);
	// This while loop is for p-prime test.
	while(primetest(p) == true){
		p = generate_k(14);
	}

	q = generate_k(15);
	// This while loop is for q-prime test.
	while(primetest(q) == true){
		q = generate_k(15);
	}

	n = p*q;
	phi_num = (p-1)*(q-1);

	local_e = generate_e();
	// This while loop is used to test valid e, gcd(e,phi) = 1.
	while(gcd_euclid_fast(local_e,phi_num) != 1){
		local_e = generate_e();
	}

	d = reduce_mod(generate_d(local_e,phi_num),phi_num);
	while(mulmod(local_e,d,phi_num) != 1){
		d = reduce_mod(generate_d(local_e,phi_num),phi_num);
	}
	Serial.println("Keys generation successed.");

}

int main(){
	/*Any code indented under this line will only be run
    when the program is called directly from the terminal
    using "make","make upload" and "serial-mon".

    Args:
     	None
     Return:
     	0(None)
	*/
	setup();
	bool server_client;
	uint32_t e;
	uint32_t local_e;
	uint32_t n;
	uint32_t d;
	uint32_t m;
	uint32_t p;
	uint32_t q;
	uint32_t ckey;
	uint32_t cmod;
	uint32_t skey;
	uint32_t smod;
	uint32_t phi_num;
	uint8_t CR = 'C';
	uint8_t ACK = 'A';
	bool communication = false;
	// Generate all necessary keys and modulus for communication.
	Generate_keys_for_server_and_client(p,q,n,phi_num,local_e,d);
	// Communication part.
	if (digitalRead(portpin) == HIGH){
		server_client = true;
		Serial.println("I'm server.");
		skey = local_e;
		smod = n;
		server_action(CR,ACK,skey,smod,ckey,cmod);
		e = ckey;
		m = cmod;
		communication = true;
	}
	else{
		server_client = false;
		Serial.println("I'm client.");
		ckey = local_e;
		cmod = n;
		client_action(CR,ACK,ckey,cmod,skey,smod);
		e = skey;
		m = smod;
		communication = true;
	}
	Serial.print("ckey: ");
	Serial.println(ckey);
	Serial.print("skey: ");
	Serial.println(skey);
	while(communication){
		uint32_t num;
		if (Serial.available() > 0){
			uint32_t we_read = Serial.read();
			if (we_read == '\r'){
				Serial.print("\n");
				uint32_to_serial3(Encryption('\r', e, m));
				uint32_to_serial3(Encryption('\n', e, m));
			}
			else{
				num = Encryption(we_read, e, m);
				uint32_to_serial3(num);
			}
			// We wrte the thing to server's serial1 so we don't need
			// to turn on the local echo.
			Serial.write(we_read);
		}
		if (Serial3.available() > 3){
			uint32_t num1 = uint32_from_serial3();
			Serial.write(Decryption(num1, d, n));
		}
	}
	Serial.flush();
	return 0;
}
