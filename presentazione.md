# HTTP - HTTPS

## GET - HEAD

### NOT CHUNKED

- `./wannabeCurl 'http://www.unimi.it/'` (301)
- `./wannabeCurl 'http://www.unimi.it/it'`
- `./wannabeCurl 'http://www.httpbin.org/'`
- `./wannabeCurl 'http://www.httpbin.org/get'`

### CHUNKED

- `./wannabeCurl 'http://www.google.com'`

## POST - PUT - DELETE

### NOT CHUNKED

- `./wannabeCurl 'http://www.httpbin.org/post' -m post -f key1=value1 -f 'key2=value2  '`

- `./wannabeCurl 'http://www.httpbin.org/post' -m post -t 'This is a test!'`

- `./wannabeCurl 'http://www.httpbin.org/post' -m post -j '{"testKey": "testValue"}'`
