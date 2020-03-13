# s3client.vc
vc2008 S3 http client  

## curlclient
A curl http client to upload file  

## httpclient
A winhttp client to download file  

## shell
```sh
file=/tmp/aaaa.txt
objname="testfile2.txt"
bucket=test
url="https://object.ecstestdrive.com/test"
resource="/${bucket}/${objname}"
contentType="application/x-compressed-tar"
dateValue=`date -R -u`
echo $dateValue;
stringToSign="PUT\n\n${contentType}\n${dateValue}\n${resource}"
s3Key="131085564036648177@ecstestdrive.emc.com"
s3Secret="gNOr3iIGAye41Efetaf7E0yI9R9KZRYZV9c3OP69"
signature=`echo -en ${stringToSign} | openssl sha1 -hmac ${s3Secret} -binary | base64`
curl -v -X PUT -T "${file}" \
  -H "Date: ${dateValue}" \
  -H "Content-Type: ${contentType}" \
  -H "Authorization: AWS ${s3Key}:${signature}" "https://object.ecstestdrive.com/${bucket}/${objname}"
```
