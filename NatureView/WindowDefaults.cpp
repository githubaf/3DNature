// WindowDefaults.cpp

#include <cctype>
#include <algorithm>

#include "WindowDefaults.h"
#include "MiscStringUtil.h"


// all windows default to on
static bool _ShowInfoWindowByDefault = true, _ShowNavWindowByDefault = true, _ShowDriveWindowByDefault = true, _ShowMapWindowByDefault = true, _ShowViewFullscreenByDefault = true;
static ToolWindowLocation _ToolWindowLocation = Bottom;

ToolWindowLocation GetToolWindowLocation(void)
{
return(_ToolWindowLocation);
} // GetToolWindowLocation

bool GetShowInfoWindowByDefault(void)
{
return(_ShowInfoWindowByDefault);
} // GetShowInfoWindowByDefault

bool GetShowNavWindowByDefault(void)
{
return(_ShowNavWindowByDefault);
} // GetShowNavWindowByDefault

bool GetShowDriveWindowByDefault(void)
{
return(_ShowDriveWindowByDefault);
} // GetShowDriveWindowByDefault

bool GetShowMapWindowByDefault(void)
{
return(_ShowMapWindowByDefault);
} // GetShowMapWindowByDefault

bool GetShowViewFullscreenByDefault(void)
{
return(_ShowViewFullscreenByDefault);
} // GetShowViewFullscreenByDefault

size_t ParseWindowDefaultOptions(const char *DefOpts)
{
int NumParsed = 0;
std::vector<std::string> TokenSet;

Tokenize(DefOpts, TokenSet, "|");

// handle each token
for(std::vector<std::string>::iterator TokenWalk = TokenSet.begin(); TokenWalk != TokenSet.end(); TokenWalk++)
	{
	const char *TokenName = NULL; // we don't use a real string so we avoid making a copy

	// UPPERCASE it for easier insensitive comparisons
	std::transform((*TokenWalk).begin(), (*TokenWalk).end(), (*TokenWalk).begin(), (int(*)(int)) toupper);

	bool NegativeForm = false;
	// is it the negative form?
	if((*TokenWalk).length() > 1 && (*TokenWalk)[0] == '-')
		{
		NegativeForm = true;
		TokenName = (*TokenWalk).c_str();
		TokenName = &TokenName[1]; // advance over one char
		} // if
	else if((*TokenWalk).length() > 2 && (*TokenWalk)[0] == 'N' && (*TokenWalk)[1] == 'O')
		{
		NegativeForm = true;
		TokenName = (*TokenWalk).c_str();
		TokenName = &TokenName[2]; // advance over two chars
		} // else if
	else
		{
		// leave it intact, but transfer over to TokenName for common processing, below
		TokenName = (*TokenWalk).c_str();
		} // else
	
	// see if the rest of it (already UPPERCASE) is recognizable as a window word we understand
	if(TokenName)
		{
		if(!strcmp(TokenName, "INFO"))
			{
			_ShowInfoWindowByDefault = !NegativeForm;
			NumParsed++;
			} // if
		else if(!strcmp(TokenName, "DRIVE"))
			{
			_ShowDriveWindowByDefault = !NegativeForm;
			NumParsed++;
			} // if
		else if(!strncmp(TokenName, "NAV", 3)) // NAVigation short form acceptible
			{
			_ShowNavWindowByDefault = !NegativeForm;
			NumParsed++;
			} // if
		else if(!strcmp(TokenName, "MAP"))
			{
			_ShowMapWindowByDefault = !NegativeForm;
			NumParsed++;
			} // if
		else if(!strcmp(TokenName, "VIEW"))
			{
			_ShowViewFullscreenByDefault = !NegativeForm;
			NumParsed++;
			} // if
		else if(!strcmp(TokenName, "TOP"))
			{ // ignores "-" or "NO" prefix
			_ToolWindowLocation = Top;
			NumParsed++;
			} // if
		else if(!strcmp(TokenName, "BOTTOM"))
			{ // ignores "-" or "NO" prefix
			_ToolWindowLocation = Bottom;
			NumParsed++;
			} // if
		else if(!strcmp(TokenName, "LEFT"))
			{ // ignores "-" or "NO" prefix
			_ToolWindowLocation = Left;
			NumParsed++;
			} // if
		else if(!strcmp(TokenName, "RIGHT"))
			{ // ignores "-" or "NO" prefix
			_ToolWindowLocation = Right;
			NumParsed++;
			} // if
		} // if


	} // for

return(TokenSet.size()); // number parsed
} // ParseWindowDefaultOptions