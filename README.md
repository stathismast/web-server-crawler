# Web Server, Web Crawler in C & Website Creator in Bash

- The website creator script in Bash is designed to create a random website from the contents of files in a given directory. The number of pages created can be controlled and every page has a number of links to another page.
- The web server and web crawler are implemented in C. The server is designed to serve the pages created by the website creator script. It assignes a new thread - from a pool of available threads - to server every new HTTP request.
- The web crawler is designed to crawl through webpages by storing every link it finds in each page and assigning a new thread to crawl through one of the links found earlier. If a link is found more than once it is ignored. The crawler also stores every website it visits. That way, if we set up the crawler to begin exploring the pages created by the Website Creator script though the web server described above, by the end of the crawling it will have made an exact copy of the pages created by the script.

## Dependencies
- In order to run this code you only need to have ```make``` and the ```gcc``` compiler installed on your machine.
- If by any chance those are not installed on your Linux distro, you can install them as such:
```
sudo apt-get install make
sudo apt-get install gcc
```

## Compile

- To compile the server and crawler, execute:
```
make
```
from the corresponding directories.

## Execute

-  To execute, run the executable created from the compilation and make sure to include all the required parameters. Don't worry, if you make a mistake a relevant message will appear to let you know what you need to add.
