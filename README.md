# wannabeCurl

## Installation

### Install dependencies

`sudo apt update; sudo apt install build-essential manpages-dev libssl-dev openssl`

### Build wannabeCurl

`make clean wannabeCurl`

## Run

```
Usage: wannabeCurl [OPTION...] URL

  -f, --form='key=value'     Add an html form body, can be used multiple times
                             to add multiple key value pairs
  -h, --header='name: value' Add the name value pair as header to the request,
                             can be used multiple times.
  -j, --json='json string'   Add a json body to the request.
                             It also add the header with the correct encoding.
  -m, --method=METHOD        Choose the method of the HTTP/S request.
                             Methods available GET (default), HEAD, OPTIONS,
                             POST, PUT, DELETE
  -q, --quiet                Suppress all console output except errors
  -t, --text='content'       Add a text body to the request
  -v, --verbose              Enable verbose console output
  -?, --help                 Give this help list
      --usage                Give a short usage message

Mandatory or optional arguments to long options are also mandatory or optional
for any corresponding short options.
```
