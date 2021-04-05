#include <openssl/evp.h>
#include <iostream>
#include <sstream>
#include <cmath>
#include <cstring>
#include <fstream>

// Co jsme si dnes odnesli:
// - kompilator neni kamarad, ale zakerna potvora, ktera ma tendence 18bajtovy struct zaokrouhlit na 20 bajtu a pak kvuli toho sizeof() nacita vic, nez by mel a cele se to dodrbe. Takze si v podstate stejne nepomuzes, kdyz si nacitanim structu z binarky chces usetrit trochu casu a prehlednosti
// - kdyz se tohle deje, tak i zdanlive neskodne veci jako string a += "progtest" hazou segfault, stehjne jako string a += argv[3] a problem tu neni cstring
// - Neni dobry napad mit otevreny stejny soubor jednou na cteni a jednou na zapis, byt omylem. Nikdo te neupozorni a pak se muzes divit, proc sakra tellg() na inFilu vraci -1 a proc ma najednou ten vstupni soubor 0 bajtu
// - nestrkej tam vice fancy hvezdicek a ampresandu (char*) & , nez je treba. Pokud je to samo o sobe dynamicky alokovane pole charu, zadne pretypovani ani & tam nepatri a jenom generuje dalsi segfaulty
// - v c++ je to delete[] a ne free
// - neni dobry napad kombinovat veci jako stream.read a fread.
// - neni dobry napad tyhle srandy delat vecer pred deadlinem


// inspired by https://stackoverflow.com/questions/12261915/how-to-throw-stdexceptions-with-variable-messages
// Not really needed, but I like it

class ExceptionFormatter {

public:
    template < typename Type >
    ExceptionFormatter & operator << ( const Type & value ) {
        stream_ << value;
        return * this;
    }
    std :: string str() const {
        return stream_.str();
    }

    operator std :: string() const {
        return stream_.str();
    }
    enum ConvertToString {
        to_str
    };

    std :: string operator >> ( ConvertToString ) {
        return stream_.str();
    }

private:

    std :: stringstream stream_;

};

struct ImageHeader {

    uint8_t     lengthOfImageId;
    uint8_t     colorMapType;
    uint8_t     imageType;
    uint16_t    startColorMap;
    uint16_t    lengthOfColorMap;
    uint8_t     bitDepth;
    unsigned char imageSpecification [ 10 ];

} __attribute__((packed)); // To force compiler to use 1 byte packaging

int main ( int argc, char * argv [ ] ) {

    // requirements for execution
    // ./block ACTION MODE FILENAME
    //  ACTION   = {e,d}          encrypt / decrypt
    //  MODE     = {ecb,cbc,...}  operational mode of the block cipher
    //  FILENAME = arg            file to de/encrypt

    if ( argc != 4 ) {
        throw std :: invalid_argument (
                ExceptionFormatter() << "Wrong number of arguments.\n"
                        << "Usage: ./block ACTION={e,d} MODE={ecb,cbc,...} FILENAME\n"
                        << "e.g. ./block e cbc file_to_encrypt.txt"
        );
    }
    /// @var flag for EVP_Cipher: true = encrypt, false = decrypt
    bool encrypt;

    if( argv [ 1 ][ 0 ] == 'e' )
        encrypt = true;
    else if ( argv [ 1 ][ 0 ] == 'd' )
        encrypt = false;
    else {
        throw std :: invalid_argument( ExceptionFormatter()
            << "Unknown argument ACTION={e,d}: reveiced: '"
            <<  argv [ 1 ]  << "'"
        );
    }

    if( std :: strcmp( argv [ 2 ] , "ecb" ) != 0 && std :: strcmp( argv [ 2 ] , "cbc" ) != 0 ) {
        throw std :: invalid_argument( ExceptionFormatter()
                                            << "Unknown argument MODE={ecb,cbc}: reveiced: "
                                            << "'" <<  argv [ 2 ] << "'" );
    }

    // We are supposed to read and copy unencrypted header and based on header we should figure out imageData length
    // and encrypt/decrypt them based on the mode in bool 'encrypt'

    // read the header
    std :: string fileInName( argv [ 3 ] );
    std :: ifstream fileIn;

    fileIn.open( fileInName, std :: ifstream :: in | std :: ifstream :: binary );

    if( !fileIn ) {
        throw std :: invalid_argument(ExceptionFormatter()
                                            << "Unable to open/read the input file. "
                                            << "Do you have right permissions?");
    }

    std :: ofstream fileOut;
    std :: string fileOutName = fileInName; // modify later

    if ( encrypt ) {
        fileOutName += "_";
        fileOutName += argv [ 2 ];
        fileOutName += ".tga";
        fileOut.open ( fileOutName, std :: ofstream :: out | std :: ofstream :: binary );
    }
    else {
        fileOutName = fileInName + "_dec.tga";
        fileOut.open( fileOutName, std :: ofstream :: out | std :: ofstream :: binary );
    }

    // read struct ImageHeader which should (if compliller respacts 1b padding

    ImageHeader ih {};

    // Position	Length [B]  Meaning
    // 0    	1	        Length of imageID
    // 1	    1	        Color map type
    // 2	    1       	Image type
    // 3	    2	        Start of color map
    // 5    	2       	Len of color map
    // 7    	1       	Bit length of color map
    // 8    	10      	Image specification --------- ImageHeader class ends here
    // 18   	…​	        Image identification (keep as is)
    //                      Color map (keep as is)
    // x	    till end    Image data

    fileIn.read( ( char * ) & ih, sizeof( ImageHeader ) );

    // now based on data in ImageHeader we know how many bytes to allocate for imageId and colorMap

    // combination of imageId and colorMap
    char * imageIdColorMap = new char [ ih.lengthOfImageId + ih.lengthOfColorMap ];
    fileIn.read ( imageIdColorMap, ih.lengthOfImageId + ih.lengthOfColorMap );

    // the rest is imageData. Read by 1024B blocks and en/decrypt to the output file

    if( !fileOut )
        throw std :: invalid_argument (
                ExceptionFormatter()
                << "Unable to open the output file. "
                << "Do you have right permissions? "
                << "Is there enough of space left on the device? "
        );
    // write binary representation of ImageHeader fo outfile
    fileOut.write ( ( char * ) &ih, sizeof( ImageHeader ) );

    fileOut.write ( imageIdColorMap, ih.lengthOfImageId + ih.lengthOfColorMap );

    // do the en/decrypt stuff

    // key for encryption
    unsigned char key[ EVP_MAX_KEY_LENGTH ] = "When I was younger, I thought programming is fun. That was ";
    // init vector
    unsigned char iv [ EVP_MAX_IV_LENGTH ] = "before progtest";

    std :: string cipherName = "aes-128-";
    cipherName += argv [ 2 ]; 

    const EVP_CIPHER * cipher;
    OpenSSL_add_all_ciphers();

    cipher = EVP_get_cipherbyname( cipherName.c_str() );

    if ( !cipher ) {
        throw std :: invalid_argument (
                ExceptionFormatter()
                    << "Cipher '" << cipherName
                    << " not found."
        );
    }
    // context structure
    EVP_CIPHER_CTX * ctx;
    ctx = EVP_CIPHER_CTX_new();
    if ( ctx == NULL )
        throw std :: invalid_argument( ExceptionFormatter() << "Unable to initialize context structure" );

    // Encrypt when 'encrypt' == true, otherwise decrypt
    // context init - set cipher, key, init vector
    if ( !EVP_CipherInit_ex ( ctx, cipher, NULL, key, iv, encrypt ) )
        throw std :: runtime_error ( "EVP_CipherInit_ex failure" );

    // Allow enough space in output buffer for additional block
    unsigned char inBuffer [ 1024 ], outBuffer [ 1024 + EVP_MAX_BLOCK_LENGTH ];
    int inLen, outLen;

    int cryptoReadStart = fileIn.tellg();  // here should start data to encrypt/decrypt

    // this might not be the ideal way, but I need to find how many bytes are remaining till the end of
    // the input file

    fileIn.seekg( 0, std :: ifstream :: end );
    int cryptoReadTotal = fileIn.tellg(); // total len of input file

    // why I need to close the stream and reopen it ?
    fileIn.close();
    fileIn.open ( fileInName, std :: ifstream :: in | std :: ifstream :: binary );

    fileIn.seekg ( cryptoReadStart, std :: ifstream :: cur ); // get back to the position before

    if ( fileIn.is_open() && fileOut.is_open() )
    {
        while ( floor(( cryptoReadTotal - cryptoReadStart ) / 1024 ) >= 0 ){
            // read 1024 bytes, there are >=1024 bytes left in the instream
            if ( (  cryptoReadTotal - cryptoReadStart ) > 1024 )
                inLen = 1024; // read this amount of bytes
            else
                inLen = cryptoReadTotal - cryptoReadStart; // read the rest of bytes

            fileIn.read ( ( char * ) inBuffer, inLen );

            if ( !EVP_CipherUpdate ( ctx, outBuffer, &outLen, inBuffer, inLen ) ) {
                EVP_CIPHER_CTX_free( ctx );
                throw std :: runtime_error( "Unable to finish EVP_CipherUpdate" );
            }

            fileOut.write ( ( char * ) outBuffer, outLen );

            cryptoReadStart += inLen;

            if ( cryptoReadStart == cryptoReadTotal )
                break;
        }


        if ( !EVP_CipherFinal_ex ( ctx, outBuffer, &outLen ) ) {
            EVP_CIPHER_CTX_free( ctx );
            if ( encrypt )
                throw std :: runtime_error ( "Encrytion failure" );
            else throw std :: runtime_error ( "Decryption failure" );
        }
        fileOut.write ( ( char * ) outBuffer, outLen );

        EVP_CIPHER_CTX_free ( ctx );
        delete [] imageIdColorMap;
        fileIn.close();
        fileOut.close();

        return 0;
    }
    throw std :: ios_base :: failure( "In/out file is not opened" );
}
