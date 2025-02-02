package com.example.ass4.ui.navigation

import androidx.compose.runtime.Composable
import androidx.navigation.NavType
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.rememberNavController
import androidx.navigation.navArgument
import com.example.ass4.ui.screen.MainScreen
import com.example.ass4.ui.screen.SearchResult
import com.example.ass4.viewmodel.PortfolioViewModel
import com.example.ass4.viewmodel.CompanyViewModel

@Composable
fun MainNavGraph() {
    val navController = rememberNavController()
    NavHost(navController = navController, startDestination = "mainScreen") {//from ChatGPT
        composable("mainScreen") {
            MainScreen(navController)
        }
        composable(
            route = "searchResult/{displaySymbol}",
            arguments = listOf(navArgument("displaySymbol") { type = NavType.StringType })  //from ChatGPT
        ) { backStackEntry ->                                                                       //from ChatGPT
            SearchResult(navController, backStackEntry.arguments?.getString("displaySymbol"))
        }
    }
}