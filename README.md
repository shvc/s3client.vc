# s3client.vc
vc2008 S3 http client  

## curlclient
A curl http client to upload file  

## httpclient
A winhttp client to download file  

## shell
```sh
#!/bin/sh

s3Key="object_user1"
s3Secret="ChangeMe"

bucket=test
objname="testfile2.txt"
file=/tmp/aaaa.txt

url="https://object.ecstestdrive.com/test"
resource="/${bucket}/${objname}"

contentType="application/x-compressed-tar"

dateValue=`date -R -u`
echo $dateValue;
stringToSign="PUT\n\n${contentType}\n${dateValue}\n${resource}"
signature=`echo -en ${stringToSign} | openssl sha1 -hmac ${s3Secret} -binary | base64`

curl -v -X PUT -T "${file}" \
  -H "Date: ${dateValue}" \
  -H "Content-Type: ${contentType}" \
  -H "Authorization: AWS ${s3Key}:${signature}" "https://object.ecstestdrive.com/${bucket}/${objname}"
```
