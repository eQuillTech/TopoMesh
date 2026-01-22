//
//  main.m
//  TriCalc
//
//  Created by Phil Ahrenkiel on 4/26/23.
//

#import <Foundation/Foundation.h>
#import "TriCalc.h"

NSString *imageName=@"topoe.tif";
NSString *fileName=@"mars.obj";
	
int main(int argc, const char * argv[]) {
#ifdef TARGET_IOS
	std::cout<<"iOS\n";
#endif

#ifdef TARGET_TVOS
	std::cout<<"TVOS\n";
#endif

#ifdef TARGET_MACOS
	std::cout<<"MacOS\n";
#endif

	NSString *folder=[NSString stringWithFormat:@"%@/%@",NSHomeDirectory(),@"TopoData"];
	NSString *imagePath=[NSString stringWithFormat:@"%@/%@",folder,imageName];

	TopoTri* topoTri=[TriCalc makeTri:imagePath];
	if(!topoTri)
		return -1;
	
	NSString *filePath=[NSString stringWithFormat:@"%@/%@",folder,fileName];
	[TriCalc writeObj:topoTri file:filePath];
	
	delete topoTri;
	return 0;
}
