PROGRAMDIR = Program

.PHONY: all docs clean buildDirs

all: 
	make -f makefileBack && make -f makefileFront && make -f makefileBackFront && make -f makefileMiddle
	cp backBuild/bin/backEnd 			$(PROGRAMDIR)/backEnd
	cp frontBuild/bin/frontEnd 			$(PROGRAMDIR)/frontEnd 
	cp middleBuild/bin/middleEnd 		$(PROGRAMDIR)/middleEnd  	
	cp backFrontBuild/bin/backFrontEnd	$(PROGRAMDIR)/backFrontEnd

clean:
	make -f makefileBack clean && make -f makefileFront clean && make -f makefileBackFront clean && make -f makefileMiddle clean

buildDirs:
	make -f makefileBack buildDirs && make -f makefileFront buildDirs && \
	make -f makefileBackFront buildDirs && make -f makefileMiddle buildDirs
