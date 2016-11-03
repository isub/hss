rm -rf CA
mkdir CA
echo 01 > CA/serial
touch CA/index.txt



# CA self certificate
openssl req  -new -batch -x509 -days 3650 -nodes -newkey rsa:1024 -out cacert.pem -keyout cakey.pem -subj /CN=ca.vbox.net/C=FR/ST=BdR/L=Aix/O=fD/OU=HSS

openssl genrsa -out hss.key.pem 1024
openssl req -new -batch -out hss.csr.pem -key hss.key.pem -subj /CN=hss/C=FR/ST=BdR/L=Aix/O=fD/OU=Tests
openssl ca -cert cacert.pem -keyfile cakey.pem -in hss.csr.pem -out hss.cert.pem -outdir . -batch
