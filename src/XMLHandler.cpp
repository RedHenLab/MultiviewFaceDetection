//#include "../include/XMLHandler.h"
//
//XMLHandler::XMLHandler(void)
//{
//}
//
//XMLHandler::~XMLHandler(void)
//{
//}
//int XMLHandler::save(string directory, CascadeData cc_data)
//{
//	CMarkup xml;
//	xml.AddElem( MCD_T("CASCADE") );
//	xml.IntoElem();
//	for(int i=0; i<cc_data.num_stage; i++)
//	{
//		xml.AddElem( MCD_T("LUTAda"));
//		xml.SetAttrib( MCD_T("stage"),i);//cc_data.lut_adas[i].stage);
//		xml.IntoElem();
//		for(int j=0; j<cc_data.lut_adas[i].num_total; j++)
//		{
//			xml.AddElem( MCD_T("LUTCLASSIFIER"));
//			xml.SetAttrib( MCD_T("l_num"),j);
//			xml.IntoElem();
//			if(j==0 && i!=0){	// nested
//				xml.AddElem(MCD_T("MINFEAT"),cc_data.lut_adas[i].lut_classifiers[j].dmin_featValue);
//				xml.AddElem(MCD_T("MAXFEAT"),cc_data.lut_adas[i].lut_classifiers[j].dmax_featValue);
//			}
//			else{               // not nested
//				xml.AddElem(MCD_T("MINFEAT"),cc_data.lut_adas[i].lut_classifiers[j].imin_featValue);
//				xml.AddElem(MCD_T("MAXFEAT"),cc_data.lut_adas[i].lut_classifiers[j].imax_featValue);
//			
//
//				for(int k=0; k<cc_data.lut_adas[i].lut_classifiers[j].features.size; k++)
//				{
//					xml.AddElem( MCD_T("FEATURE"));
//					xml.SetAttrib( MCD_T("f_num"),k);
//					xml.IntoElem();
//					xml.AddElem( MCD_T("ID"),cc_data.lut_adas[i].lut_classifiers[j].features.id[k]);
//					xml.AddElem( MCD_T("SIGN"),cc_data.lut_adas[i].lut_classifiers[j].features.sign[k]);
//					xml.OutOfElem();
//				}
//			}
//			xml.AddElem( MCD_T("CONFIDENCE"));
//			xml.IntoElem();
//			for(int m=0; m<K_LUTBINS; m++)
//			{
//				xml.AddElem( MCD_T("CONF"),cc_data.lut_adas[i].lut_classifiers[j].conf[m]);
//				xml.SetAttrib( MCD_T("c_num"),m);
//			}
//			
//			xml.OutOfElem();
//			
//			xml.OutOfElem();
//		}
//		xml.OutOfElem();
//	}
//	xml.OutOfElem();
//	bool result = xml.Save(s2ws(directory));
//	if(result)
//		return 0;
//	cerr<<"failed to save xml";
//	return -1;
//}
//
//int XMLHandler::load(std::string directory, CascadeData &cc_data)
//{
//	
//	CMarkup xml;
//	bool result = xml.Load(s2ws(directory));
//	if(!result){
//		cerr<<"failed to load xml";
//		return -1;
//	}
//	xml.FindElem(MCD_T("CASCADE"));
//	cc_data.num_stage=0;
//	cc_data.lut_adas.clear();
//	xml.IntoElem(); // Cascade
//	while(xml.FindElem(MCD_T("LUTAda")))
//	{
//		cc_data.num_stage++;
//		LUTAdaData lutada_data;
//		lutada_data.stage = ws2i(xml.GetAttrib(MCD_T("stage")));
//		lutada_data.num_total = 0;
//		xml.IntoElem(); // LUTAda
//
//		while(xml.FindElem(MCD_T("LUTCLASSIFIER")))
//		{
//			lutada_data.num_total++;
//			LUTClassifierData lutc_data;
//			lutc_data.num_lut = ws2i(xml.GetAttrib(MCD_T("l_num")));
//			SparseFeature sf_data;
//			sf_data.size=0;
//			xml.IntoElem(); // LUTC
//
//			if(lutc_data.num_lut==0 && lutada_data.stage!=0)	//nested
//			{  
//				xml.FindElem(MCD_T("MINFEAT"));
//				lutc_data.dmin_featValue = ws2f(xml.GetData());
//				xml.FindElem(MCD_T("MAXFEAT"));
//				lutc_data.dmax_featValue = ws2f(xml.GetData());
//			}
//			else
//			{
//				xml.FindElem(MCD_T("MINFEAT"));
//				lutc_data.imin_featValue = ws2i(xml.GetData());
//				xml.FindElem(MCD_T("MAXFEAT"));
//				lutc_data.imax_featValue = ws2f(xml.GetData());
//			
//				while(xml.FindElem(MCD_T("FEATURE")))
//				{
//					sf_data.size++;
//					xml.IntoElem(); // Feature
//					xml.FindElem(MCD_T("ID"));
//					sf_data.id.push_back(ws2i(xml.GetData()));
//					xml.FindElem(MCD_T("SIGN"));
//					sf_data.sign.push_back(ws2i(xml.GetData())!=0);
//					xml.OutOfElem(); // Feature
//				}
//				lutc_data.features = sf_data;
//			}
//
//			xml.FindElem(MCD_T("CONFIDENCE"));
//			xml.IntoElem(); // Confidence
//			for(int i=0;i<K_LUTBINS;i++)
//			{
//				xml.FindElem(MCD_T("CONF"));
//				lutc_data.conf[i] = ws2f(xml.GetData());
//			}
//			xml.OutOfElem(); // Confidence
//			lutada_data.lut_classifiers.push_back(lutc_data);
//			xml.OutOfElem(); //LUTC
//		}
//		cc_data.lut_adas.push_back(lutada_data);
//		xml.OutOfElem(); // LUTAda
//	}
//	xml.OutOfElem(); // Cascade		
//
//
//	return 0;
//}
//
//wstring XMLHandler::s2ws(const string &s)
//{
//	std::wstring w_str(s.length(), L'');
//	std::copy(s.begin(), s.end(), w_str.begin());
//	return w_str;
//}
//
//string XMLHandler::ws2s(const wstring &ws)
//{
//	std::string s_str(ws.length(), ' ');
//	std::copy(ws.begin(), ws.end(), s_str.begin());
//	return s_str;
//}
//
//int XMLHandler::ws2i(const wstring &ws)
//{
//	return atoi( ws2s(ws).c_str());
//}
//double XMLHandler::ws2f(const std::wstring &ws)
//{
//	return atof( ws2s(ws).c_str());
//}