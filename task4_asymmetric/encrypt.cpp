#include <openssl/evp.h>
#include <iostream>
#include <cstring>
#include <fstream>
#include <cstdlib>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/rand.h>
#include <string>

#define BUFFER_LEN 128

class Asymetric{
private:
    std::string pKeyPath, inputFilePath, cipherName, outputFilePath;
    // for public/private keys
    FILE * keyFile = nullptr;
    EVP_PKEY * pKey = nullptr;
    EVP_CIPHER_CTX * ctx;


    std::ifstream inputFile;
    std::ofstream outputFile;
    int cipherId = 0;
    unsigned char * inputBuffer;
    unsigned char * outputBuffer;
    int inputLength, outputLength;
    int encryptionKeyLen = 0;

    // init vector
    unsigned char iv [ EVP_MAX_IV_LENGTH ] = "bezbezbezb";

public:
    Asymetric(const std::string & pKeyPath,
            const std::string & inputFilePath,
            const std::string & outputFilePath,
            const std::string & cipherName):

            pKeyPath(pKeyPath),
            inputFilePath(inputFilePath),
            outputFilePath(outputFilePath),
            cipherName(cipherName){

        // buffers
        inputBuffer = new unsigned char[BUFFER_LEN * 2];
        inputLength = BUFFER_LEN;
        outputBuffer = new unsigned char[BUFFER_LEN * 2];
        outputLength = BUFFER_LEN;

        inputFile.open(inputFilePath, std::ifstream::in | std::ifstream::binary);
        outputFile.open(outputFilePath, std::ifstream::out | std::ifstream::binary);

        if (!inputFile.is_open() ){
            std::cerr << "Can not open input file " << inputFilePath << "\nDo you have right permissions?" <<  std::endl;
            throw  std::invalid_argument("Can not open inputFile file");
            //return false;
        }
        if (inputFile.peek() == std::ifstream::traits_type::eof()){
            std::cerr << "Input file is empty" << std::endl;
            throw std::invalid_argument("Can not encrypt/decrypt empty file");
        }
        if (!outputFile.is_open()){
            std::cerr << "Can not open output file " << outputFilePath  << "\nDo you have right permissions?" << std::endl;
            throw std::invalid_argument("Can not open outputFile file");
        }

        keyFile = fopen(pKeyPath.c_str(), "r");
        if (!keyFile) {
            std::cerr << "invalid private key. For encrypt use publickey, for decrypt use privatekey" << std::endl;
            throw std::invalid_argument("Unable to read public/private key. For encrypt use publickey");
        }
        ctx = EVP_CIPHER_CTX_new();
        if(!ctx){
            std::cerr << "Unable to create context" << std::endl;
            throw std::runtime_error("Unable to create context");
        }

        OpenSSL_add_all_ciphers();

    };
    ~Asymetric(){
        delete [] outputBuffer;
        delete [] inputBuffer;
    }
    void clear(bool wasCryptSuccessfull = false){
        fclose(keyFile);
        EVP_PKEY_free(pKey);
        EVP_CIPHER_CTX_free(ctx);
        inputFile.close();
        outputFile.close();
        if(!wasCryptSuccessfull)
            std::remove(outputFilePath.c_str());
    }


    int encrypt(){

        if(!(pKey = PEM_read_PUBKEY(keyFile, NULL, NULL, NULL))){
            std::cerr << "Unable to read public key" << std::endl;
            return 1;
        }

        auto * encryptionKey = (unsigned char *) malloc (EVP_PKEY_size(pKey));

        const EVP_CIPHER * cipher = EVP_get_cipherbyname(cipherName.c_str());
        if ( !cipher ) {
            std :: cerr << "Cipher not found.";
            return 2;
        }
        cipherId = EVP_CIPHER_nid(cipher);

        // use only 1 public key
        // send type, my_ek, my_ekl, iv to the recipient with the ciphertext

        if(!EVP_SealInit(ctx, cipher, &encryptionKey, &encryptionKeyLen, iv, &pKey, 1)){
            std :: cerr << "Can not run EVP_SealInit";
            return 3;
        }
        // write cipher id to output file
        outputFile.write((char *) &cipherId, sizeof(cipherId));

        // key
        outputFile.write((char *) &encryptionKeyLen, sizeof(encryptionKeyLen));
        outputFile.write((char *) encryptionKey, encryptionKeyLen);
        // vector

        outputFile.write((char *) iv, EVP_MAX_IV_LENGTH);

        // data itself
        inputFile.read((char *) inputBuffer, inputLength);
        inputLength = inputFile.gcount();

        bool success = true;
        while(inputLength){
            if(!EVP_SealUpdate(ctx, outputBuffer, &outputLength, inputBuffer, inputLength)){
                success = false;
                break;
            }
            int len = outputLength;

            if(inputLength < BUFFER_LEN){
                if(!EVP_SealFinal(ctx, outputBuffer + outputLength, &outputLength)){
                    success = false;
                    break;
                }
                len += outputLength;
            }

            outputFile.write((char *) outputBuffer, len);
            inputFile.read((char *) inputBuffer, inputLength);

            // Returns the number of characters extracted by the last unformatted input operation performed on the object.
            inputLength = inputFile.gcount();
        }

        // encryption failed, no files should be created
        free(encryptionKey);
        clear(success); // success == false -> remove outfile if created

        return success ? 0 : 4; // return 4 error code if failed
    }

    int decrypt(){

        // read private key
        pKey = PEM_read_PrivateKey(keyFile, NULL, NULL, NULL);
        if(!pKey){
            std::cerr << "Unable to read pKey" << std::endl;
            return 6;
        }

        // read cipher type, encryption key len, encryption key, IV from encrypted file

        // read cipher id
        inputFile.read((char *) &cipherId, sizeof(cipherId));

        const EVP_CIPHER *cipher = EVP_get_cipherbynid(cipherId);
        if (!cipher) {
            std::cerr << "Cipher not found.";
            return 7;
        }
        // length of encryption key
        inputFile.read((char *) &encryptionKeyLen, sizeof(encryptionKeyLen));
        if(!inputFile){
            std::cerr << "Unexpected ond of file while loading header data" << std::endl;
            return 8;
        }
        // alloc space for encryptionKey
        auto * encryptionKey = (unsigned char *) malloc(encryptionKeyLen);

        inputFile.read(( char *) encryptionKey, encryptionKeyLen);
        if(!inputFile){
            std::cerr << "Unexpected ond of file while loading header data" << std::endl;
            return 8;
        }
        // vector
        inputFile.read((char *) &iv, EVP_MAX_IV_LENGTH);
        if(!inputFile){
            std::cerr << "Unexpected ond of file while loading header data" << std::endl;
            return 8;
        }

        // use only 1 public key
        // send type, my_ek, my_ekl, iv to the recipient with the ciphertext
        if(1 != EVP_OpenInit(ctx, cipher, encryptionKey, encryptionKeyLen, iv, pKey)){
            std::cerr << "Unable to OpenInit" << std::endl;
            return 9;
        }

        // data itself
        inputFile.read((char *) inputBuffer, inputLength);
        inputLength = inputFile.gcount();

        bool success = true;
        while(inputLength){
            if(!EVP_OpenUpdate(ctx, outputBuffer, &outputLength, inputBuffer, inputLength)){
                success = false;
                break;
            }
            int len = outputLength;

            if(inputLength < BUFFER_LEN){
                if(!EVP_OpenFinal(ctx, outputBuffer + outputLength, &outputLength)){
                    success = false;
                    break;
                }
                len += outputLength;
            }

            outputFile.write((char *) outputBuffer, len);
            inputFile.read((char *) inputBuffer, inputLength);

            // Returns the number of characters extracted by the last read
            inputLength = inputFile.gcount();
        }
        free(encryptionKey);

        clear(success);
        return success ? 0 : 11; // return 11 error code if failed
    }
};


int main ( int argc, char * argv [ ] ) {

    // Zadani a pozadavky
    // ==============
    // binárka se bude jmenovat encrypt a brát 4 poziční parametry v následující formě:
    //
    //./encrypt ACTION PKEY FILEIN FILEOUT
    // ACTION      = {e,d}      má-li program provést šifrování (encrypt) nebo dešifrování (decrypt)
    // PKEY        = arg        cesta ke {veřejnému, soukromému} klíči
    // FILEIN      = arg        cesta ke vstupnimu souboru
    // FILEOUT     = arg        jméno výstupního souboru
    // vytvořte si vlastní format hlavičky, který bude obsahovat všechny nutné údaje pro dešifrování
    //💀 návratové hodnoty: nula pro úspěch, 1 až 127 pro neúspěch.
    //💀 kontrolujte výstupy funkcí (čtení souboru, šifrování, dešifrování…​)
    //💀 dešifrovaný soubor MUSÍ být stejný jako původní
    //💀 nezapomínejte, že velikost zašifrovaného klíče je závislá na velikosti klíče asymetrického, tzn. není známá předem
    //💀 nenačítejte celý soubor do paměti
    // [-1b] nastane-li chyba, výstupní soubor se nevytvoří (žádný!)
    // [+1b] pátý nepovinný poziční parametr pro šifrování:
    //      ./encrypt e publickey filein fileout CIPHERNAME
    //      CIPHERNAME  = arg        jméno symetrické šifry ve formátu, jak to bere EVP_get_cipherbyname
    //

    if ( argc < 5 || argc > 6 ) {
        std :: cerr << "Wrong number of arguments.\n"
                    << "Usage: ./encrypt ACTION={e,d} PKEY FILEIN FILEOUT [CIPHERNAME]"
                       "    ./encrypt ACTION PKEY FILEIN FILEOUT [CIPHERNAME]\n"
                       "    - ACTION      = {e,d}      Action e = encrypt, d = decrypt\n"
                       "    - PKEY        = arg        path to {public (for encrypt) / private (for encrypt) key\n"
                       "    - FILEIN      = arg        path to input file\n"
                       "    - FILEOUT     = arg        path to output file\n"
                       "    - [CIPHERNAME]  = [arg]    name and type of cipher compatible with EVP_get_cipherbyname \n";
        return 1;
    }

    std :: string pkey = argv [ 2 ] ;
    std :: string fileInName( argv [ 3 ] );
    std :: string fileOutName = argv [ 4 ] ;

    std :: string cipherName = "aes-128-cbc";
    if(argc == 6)
        cipherName = argv [ 5 ] ;

    try {

        Asymetric asymetric(pkey, fileInName, fileOutName, cipherName);
        // this is not an elegant solution, but I wanted to save as much duplicate code as possible, thus
        // there is o lot of code in constructor
        if( argv [ 1 ][ 0 ] == 'e' )
            return asymetric.encrypt();
        else if ( argv [ 1 ][ 0 ] == 'd' )
            return asymetric.decrypt();
        else {
            std :: cerr << "Unknown argument ACTION={e,d}: reveiced: '" <<  argv [ 1 ]  << "'";
            return 2;

        }
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return 100;
    }  catch (const std::invalid_argument& e) {
        std::cerr << e.what() << std::endl;
        return 101;
    }
}
