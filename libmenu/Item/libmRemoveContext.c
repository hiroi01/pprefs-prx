#include "common.h"

void libmRemoveContext(MenuContext* Context)
{
	if( !Context || !IS_OPT_FUNC_F(Context->opt) ) return;
	
	MenuItem* Curr = Context->Root;
	
	
	//メニュー全アイテムを削除(メモリ解放）
	while( Curr )
	{
		MenuItem *last = Curr;
		
		if ( Curr->Type == MenuContainer && Curr->Children )
		{
			Curr = Curr->Children;
		}
		else
		{
			Curr = Curr->Next;
			
			if (!Curr && last->Parent)
			{
				Curr = last->Parent->Next;
			}
		}
		
		Context->opt->func.free_p(last);
	}
	
	//コンテキスト削除（メモリ解放）
	Context->opt->func.free_p(Context);
}
