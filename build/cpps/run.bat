call VsMSBuildCmd.bat
msbuild "G:\HalfPeopleStudioC++ Porject\HalfPeopel C++ Blueprint\x64\Release\build\cpps\ProJect.vcxproj" /p:OutDir="G:\HalfPeopleStudioC++ Porject\HalfPeopleBlueprintSystemTestProjectFolder\Export" /p:ProductName=$productName;AssemblyName="HalfBlueprint" /p:configuration=release /p:platform=x64
