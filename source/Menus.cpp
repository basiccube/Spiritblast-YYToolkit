#include "Menus.h"
#include "Variables.h"
#include "Functions.h"

void ChangeMenuPage(RValue menu, string name, int addToHistory)
{
	RValue changePage = GetInstanceVariable(menu, "changePage");
	if (changePage.IsUndefined())
		return;

	RValue pageValue = GetInstanceVariable(menu, name);
	if (pageValue.IsUndefined())
	{
		Print("Invalid page name!");
		return;
	}

	vector<RValue> argArray = {pageValue};
	if (addToHistory != -1)
		argArray.push_back(addToHistory);

	CallMethod(changePage, argArray);
}

RValue CreateMenuPage(RValue menu)
{
	RValue pageCreate = GetInstanceVariable(menu, "pageCreate");
	if (pageCreate.IsUndefined())
		return RValue();

	RValue newPage = CallMethod(pageCreate);
	return newPage;
}

void AddItemToPageValue(RValue menu, RValue page, CInstance *item, int index)
{
	if (index != -1)
		g_interface->CallBuiltin("ds_list_insert", {page, index, item});
	else
		g_interface->CallBuiltin("ds_list_add", {page, item});
}

void AddItemToPage(RValue menu, string page, CInstance *item, int index)
{
	RValue pageVar = GetInstanceVariable(menu, page);
	if (pageVar.IsUndefined())
	{
		Print(RValue("Invalid page specified: " + page));
		return;
	}

	AddItemToPageValue(menu, pageVar, item, index);
}

RValue CreateFakeMenuInstance(RValue menu)
{
	RValue defaultFont = GetInstanceVariable(menu, "defaultFont");

	// Create a fake struct consisting of the variables required
	// by the menu button since menu.ToInstance() doesn't fucking work
	map<string, RValue> fakeMenuInstanceMap;
	fakeMenuInstanceMap["id"] = menu;
	fakeMenuInstanceMap["defaultFont"] = defaultFont;
	RValue fakeMenuInstance = RValue(fakeMenuInstanceMap);

	return fakeMenuInstance;
}

CInstance *CreateMenuButton(RValue menu, string text, RValue buttonMethod, RValue params)
{
	RValue menuButton = g_interface->CallBuiltin("asset_get_index", {"menuButton"});
	if (menuButton.ToInt32() == GM_INVALID)
		return nullptr;

	RValue fakeMenuInstance = CreateFakeMenuInstance(menu);

	map<string, RValue> itemMap;
	RValue item = RValue(itemMap);

	CInstance *itemInst = item.ToInstance();
	CInstance *menuInst = fakeMenuInstance.ToInstance();

	RValue res = RValue();
	g_interface->CallBuiltinEx(res, "script_execute", itemInst, menuInst, {menuButton, RValue(text), buttonMethod, params});

	return itemInst;
}

CInstance *CreateMenuToggle(RValue menu, string text, RValue value, RValue object, RValue varName)
{
	RValue menuToggle = g_interface->CallBuiltin("asset_get_index", {"menuToggle"});
	if (menuToggle.ToInt32() == GM_INVALID)
		return nullptr;

	RValue fakeMenuInstance = CreateFakeMenuInstance(menu);

	map<string, RValue> itemMap;
	RValue item = RValue(itemMap);

	CInstance *itemInst = item.ToInstance();
	CInstance *menuInst = fakeMenuInstance.ToInstance();

	RValue res = RValue();
	g_interface->CallBuiltinEx(res, "script_execute", itemInst, menuInst, {menuToggle, RValue(text), value, object, varName});

	return itemInst;
}

CInstance *CreateChangePageButton(RValue menu, string text, RValue targetPage)
{
	RValue changePage = GetInstanceVariable(menu, "changePage");
	if (changePage.IsUndefined())
		return nullptr;

	CInstance *btn = CreateMenuButton(menu, text, changePage, targetPage);
	return btn;
}

CInstance *CreateBackButton(RValue menu, string text)
{
	RValue popPage = GetInstanceVariable(menu, "popPage");
	if (popPage.IsUndefined())
		return nullptr;

	CInstance *backBtn = CreateMenuButton(menu, text, popPage);
	return backBtn;
}