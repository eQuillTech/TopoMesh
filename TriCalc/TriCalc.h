//
//  NSObject+Converter_mm.h
//  TriCalc
//
//  Created by Phil Ahrenkiel on 4/26/23.
//

#import <Foundation/Foundation.h>
#import "TopoMesh.hpp"

@interface TriCalc:NSObject

+(TopoTri*)makeTri:(NSString*)imageName;

+(void)writeDat:(TopoTri*)topoTri file:(NSString*)fileName;
+(void)writeObj:(TopoTri*)topoTri file:(NSString*)fileName;

@end

