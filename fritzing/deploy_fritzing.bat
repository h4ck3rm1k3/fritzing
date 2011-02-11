cd C:\Users\jonathan\fritzing2
svn export https://fritzing.googlecode.com/svn/trunk/fritzing/translations ./release/translations

svn export https://fritzing.googlecode.com/svn/trunk/fritzing/bins ./release/bins

svn export https://fritzing.googlecode.com/svn/trunk/fritzing/sketches ./release/sketches

svn export https://fritzing.googlecode.com/svn/trunk/fritzing/parts ./release/parts

del .\release\translations\*.ts

