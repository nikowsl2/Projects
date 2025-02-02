package com.example.ass4.ui.screen
import android.icu.util.Calendar
import android.os.Bundle
import android.util.Log
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ArrowBack
import androidx.compose.material.icons.filled.Clear
import androidx.compose.material.icons.filled.Search
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.lifecycle.viewmodel.compose.viewModel
import com.example.ass4.ui.theme.Ass4Theme
import com.example.ass4.viewmodel.PortfolioViewModel
import com.example.ass4.viewmodel.CompanyViewModel
import com.example.ass4.ui.components.Footer
import java.text.SimpleDateFormat
import java.util.Locale
import androidx.compose.foundation.gestures.Orientation
import androidx.compose.foundation.gestures.draggable
import androidx.compose.foundation.gestures.rememberDraggableState


import androidx.compose.foundation.lazy.itemsIndexed
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.ExperimentalMaterialApi
import androidx.compose.material.rememberSwipeableState
import androidx.compose.material.swipeable
import androidx.compose.runtime.livedata.observeAsState
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.layout.onGloballyPositioned
import androidx.compose.ui.platform.LocalDensity
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.text.TextStyle
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.sp
import androidx.navigation.NavController
import com.example.ass4.R
import com.example.ass4.api.AggregatedPortfolioItem
import com.example.ass4.ui.navigation.MainNavGraph
import com.example.ass4.viewmodel.CompanyWatchModel
import com.example.ass4.viewmodel.PortfolioCompanyWatchModel
import com.example.ass4.viewmodel.PortfolioInteractModel
import com.example.ass4.viewmodel.WatchListModel
import com.example.ass4.viewmodel.WatchlistViewModel
import kotlin.math.abs
import kotlin.math.sign
import kotlinx.coroutines.delay
import kotlinx.coroutines.runBlocking

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            Ass4Theme {
                MainNavGraph()
            }
        }
    }
}

@Composable
fun MainScreen(navController: NavController) {
    val watchlistViewModel = viewModel<WatchlistViewModel>()
    val watchListModel = viewModel<WatchListModel>()
    val portfolioView = viewModel<PortfolioInteractModel>()
    val companyWatch = viewModel<CompanyWatchModel>()
    val portfolioCompanyWatchModel = viewModel<PortfolioCompanyWatchModel>()
    val viewModel = viewModel<PortfolioViewModel>()
    val companyViewModel = viewModel<CompanyViewModel>()
    val isLoading = remember { mutableStateOf(true) }
    val editWatchlistViewModel = viewModel<WatchlistViewModel>()
    val isFirstLoad = rememberSaveable { mutableStateOf(true) }


    Column(modifier = Modifier.fillMaxSize()) {
        if (isLoading.value && isFirstLoad.value) {
            SearchToggle(companyViewModel, navController)
            Box(modifier = Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
                Image(
                    painter = painterResource(id = R.drawable.launcher),
                    contentDescription = "App Logo",
                    modifier = Modifier
                        .background(Color.White)
                        .width(100.dp)
                        .height(100.dp)
                )
            }
        }

        LaunchedEffect(key1 = Unit) {
            delay(1000)
            isFirstLoad.value = false
        }


        if (isLoading.value) {
            SearchToggle(companyViewModel, navController)
            Box(modifier = Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
                CircularProgressIndicator()
            }
        }

        LaunchedEffect(key1 = Unit) {
            delay(3000)
            isLoading.value = false
        }

        LazyColumn(modifier = Modifier.alpha(if (isLoading.value) 0f else 1f)) {
            item { SearchToggle(companyViewModel, navController)}
            item { Divider(color = Color.LightGray, thickness = 2.dp) }
            item { GetDateAndTime() }
            item { PortfolioBox(viewModel, portfolioView, companyWatch) }
            item { Portfolio(portfolioView, portfolioCompanyWatchModel, navController) }
            item { FavoritesBox(watchlistViewModel, watchListModel, navController, editWatchlistViewModel) }
            item { Footer() }
            item { Spacer(modifier = Modifier.height(16.dp)) }
        }
    }
}



@Composable
fun GetDateAndTime() {
    val calendar = Calendar.getInstance().time
    val dateFormat = SimpleDateFormat("d MMMM yyyy", Locale.getDefault()).format(calendar)
    Text(text = dateFormat,
        style = TextStyle(
            color = Color.Gray,
            fontWeight = FontWeight.Bold,
            fontSize = 20.sp
        ),
        modifier = Modifier
            .padding(start = 16.dp, top = 10.dp, bottom = 10.dp, end = 16.dp)
    )
}

@Composable
fun PortfolioBox(
    viewModel: PortfolioViewModel,
    portfolioViewModel: PortfolioInteractModel,
    companyWatchModel: CompanyWatchModel) {

    val cashBalance by viewModel.cashBalance.collectAsState()
    val aggregatedPortfolioItems by portfolioViewModel.aggregatedPortfolioDetails.observeAsState(emptyList())
    val watchResult = companyWatchModel.watchResults.collectAsState().value
    val totalPortfolioValue = remember(aggregatedPortfolioItems, watchResult) {
        var totalValue = 0.0
        aggregatedPortfolioItems.forEach { item ->
            val result = runBlocking {
                companyWatchModel.watchCompany(item.ticker)
                delay(3000L)
                watchResult?.c ?: 0.0
            }
            totalValue += item.totalShares * result
        }
        totalValue
    }
    val netWorth = cashBalance?.plus(totalPortfolioValue) ?: 0.0

    Box(
        modifier = Modifier
            .fillMaxWidth()
            .background(color = Color.LightGray),
        contentAlignment = Alignment.CenterStart
    ) {
        Text(
            text = "Portfolio",
            style = MaterialTheme.typography.headlineMedium,
            modifier = Modifier
                .padding(start = 16.dp, top = 4.dp, bottom = 4.dp, end = 16.dp)
        )
    }

    Column() {
        Row(modifier = Modifier
            .fillMaxWidth()
            .padding(horizontal = 16.dp, vertical = 4.dp),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.SpaceBetween) {
            Column(modifier = Modifier.weight(1f)) {
                Text(
                    text = "Net Worth",
                    style = TextStyle(
                        fontSize = 20.sp
                    )
                )
                Spacer(modifier = Modifier.height(8.dp))
                Text(
                    text = "$${"%.2f".format(netWorth)}",
                    style = TextStyle(
                        fontSize = 20.sp
                    )
                )
            }
            Column(horizontalAlignment = Alignment.End)
            {
                Text(
                    text = "Cash Balance",
                    style = TextStyle(
                        fontSize = 20.sp
                    )
                )
                Spacer(modifier = Modifier.height(8.dp))
                Text(
                    text = "$${"%.2f".format(cashBalance)}",
                    style = TextStyle(
                        fontSize = 20.sp
                    ),
                )
            }
        }
    }
}

@Composable
fun Portfolio(
    viewModel: PortfolioInteractModel,
    portfolioCompanyWatchModel: PortfolioCompanyWatchModel,
    navController: NavController) {
    val aggregatedPortfolioItems by viewModel.aggregatedPortfolioDetails.observeAsState(initial = emptyList())

//    LaunchedEffect(Unit) {
//        viewModel.fetchTickers()
//    }
    LaunchedEffect(key1 = true) {
        while (true) {
            viewModel.fetchTickers()
            delay(30000)
            Log.d("Portfolio","updating!")
        }
    }

    val reorderedItems = remember(aggregatedPortfolioItems) {
        aggregatedPortfolioItems.toMutableStateList()
    }

    Box() {
        if (reorderedItems.isEmpty()) {
            Text("", Modifier.align(Alignment.Center))
        } else {
            AggregatedPortfolioList(reorderedItems, portfolioCompanyWatchModel, navController)
        }
    }
}

@Composable
fun AggregatedPortfolioList(
    portfolioItems: MutableList<AggregatedPortfolioItem>,
    portfolioCompanyWatchModel: PortfolioCompanyWatchModel,
    navController: NavController) {

    val maxHeight = 1000.dp
    Box(modifier = Modifier
        .fillMaxWidth()
        .heightIn(max = maxHeight)
        ) {
        Column(modifier = Modifier
            .verticalScroll(rememberScrollState())
            ) {
            portfolioItems.forEachIndexed { index, item ->
                var draggedIndex by remember { mutableStateOf(index) }
                var dragAccumulation by remember { mutableStateOf(0f) }
                AggregatedPortfolioItemView(
                    item = item,
                    modifier = Modifier.draggable(
                        orientation = Orientation.Vertical,
                        state = rememberDraggableState { delta ->
                            dragAccumulation += delta
                            val dragThreshold = 30f

                            if (abs(dragAccumulation) >= dragThreshold) {
                                val direction = if (dragAccumulation > 0) 1 else -1
                                val newIndex = (draggedIndex + direction).coerceIn(0, portfolioItems.size - 1)
                                if (newIndex != draggedIndex) {
                                    portfolioItems.move(draggedIndex, newIndex)
                                    draggedIndex = newIndex
                                }
                                dragAccumulation = 0f
                            }
                        }
                    ),
                    watchModel = portfolioCompanyWatchModel,
                    navController = navController
                )
            }
        }
    }
}

@Composable
fun AggregatedPortfolioItemView(
    item: AggregatedPortfolioItem,
    modifier: Modifier = Modifier,
    watchModel: PortfolioCompanyWatchModel,
    navController: NavController) {

    val companyProfile by watchModel.getProfileStateFlow(item.ticker).collectAsState()
    val softerGreen = Color(0xFF66BB6A)
    val softerRed = Color(0xFFF34B4B)
    val increaseIconPainter = painterResource(id = R.drawable.icons8_increase_48)
    val decreaseIconPainter = painterResource(id = R.drawable.icons8_decrease_48)

    LaunchedEffect(item.ticker) {
        watchModel.PortfolioWatchCompany(item.ticker)
    }

    Column(modifier = modifier) {
        Divider(color = Color.LightGray, thickness = 2.dp)
        Row(modifier = Modifier
            .fillMaxWidth()
            .padding(start = 16.dp, end = 4.dp)) {
            Column(modifier = Modifier.weight(1f)) {
                Row(modifier = Modifier
                    .fillMaxWidth()
                    .padding(vertical = 8.dp),
                    horizontalArrangement = Arrangement.SpaceBetween) {
                    Text(
                        text = item.ticker,
                        style = TextStyle(
                            fontWeight = FontWeight.Bold,
                            fontSize = 20.sp
                        )
                    )
                    companyProfile?.let { profile ->
                        val totalValue = profile.c * item.totalShares
                        Text(text = "$${String.format("%.2f", totalValue)}",
                            style = TextStyle(
                                fontWeight = FontWeight.Bold,
                                fontSize = 20.sp
                            )
                        )
                    } ?: Text(text = "")
                }
                Row(modifier = Modifier
                    .fillMaxWidth()
                    .padding(vertical = 4.dp),
                    horizontalArrangement = Arrangement.SpaceBetween) {
                    Text(
                        text = "${item.totalShares} shares",
                        color = Color(0xFF888888)
                    )
                    companyProfile?.let { profile ->
                        val change = (profile.c - item.averageCost) * item.totalShares
                        val changePercent = if (item.totalCost != 0.0) (change / item.totalCost) * 100 else 0.0
                        val (changeColor, iconPainter) = when {
                            change > 0 -> softerGreen to increaseIconPainter
                            change < 0 -> softerRed to decreaseIconPainter
                            else -> Color.Black to null
                        }
                        Row(verticalAlignment = Alignment.CenterVertically) {
                            if (iconPainter != null) {
                                Icon(
                                    painter = iconPainter,
                                    contentDescription = "Change Icon",
                                    tint = changeColor,
                                    modifier = Modifier
                                        .size(20.dp)
                                        .padding(end = 2.dp)
                                )
                            }
                            Text(
                                text = "\$${String.format("%.2f", change)} (${String.format("%.2f", changePercent)}%)",
                                color = changeColor,
                                style = TextStyle(
                                    fontWeight = FontWeight.Bold,
                                    fontSize = 16.sp
                                )
                            )
                        }
                    } ?: Text("")
                }
            }
            Icon(
                painter = painterResource(id = R.drawable.chevron_right),
                contentDescription = "Chevron Right",
                modifier = Modifier
                    .size(24.dp)
                    .align(Alignment.CenterVertically)
                    .clickable {
                        navController.navigate("searchResult/${item.ticker}") {
                            popUpTo("mainScreen") { inclusive = true }
                        }
                    }
            )
        }
    }
}

@Composable
fun FavoritesBox(
    watchlistViewModel: WatchlistViewModel,
    watchListModel: WatchListModel,
    navController: NavController,
    editWatchlistViewModel: WatchlistViewModel) {

    val watchlist = watchlistViewModel.watchlist.collectAsState().value
    val reorderedList = remember(watchlist) { watchlist.toMutableStateList() }
    val maxHeight = 600.dp

    LaunchedEffect(key1 = true) {
        while (true) {
            Log.d("WatchList","update!")
            watchlistViewModel.fetchWatchlist()
            delay(15000)
        }
    }

    Box(
            modifier = Modifier
                .fillMaxWidth()
                .background(color = Color.LightGray),
            contentAlignment = Alignment.CenterStart
        ) {
            Text(
                text = "Favorites",
                style = MaterialTheme.typography.headlineMedium,
                modifier = Modifier.padding(start = 16.dp, top = 4.dp, bottom = 4.dp, end = 16.dp)
            )
        }

    Box(modifier = Modifier
        .fillMaxWidth()
        .heightIn(max = maxHeight)
    ) {
        Column(modifier = Modifier.verticalScroll(rememberScrollState())) {
            reorderedList.forEachIndexed { index, tickerProfile ->
                var dragAmount by remember { mutableStateOf(0f) }
                var draggedIndex by remember(tickerProfile.ticker) { mutableStateOf(index) }
                CompanyProfileItem(
                    ticker = tickerProfile.ticker,
                    WatchlistModel = watchListModel,
                    modifier = Modifier.draggable(
                        orientation = Orientation.Vertical,
                        state = rememberDraggableState { delta ->
                            dragAmount += delta
                            val dragThreshold = 200f
                            if (abs(dragAmount) >= dragThreshold) {
                                val newIndex = (draggedIndex + sign(dragAmount).toInt()).coerceIn(0, reorderedList.size - 1)
                                if (newIndex != draggedIndex) {
                                    reorderedList.move(draggedIndex, newIndex)
                                    draggedIndex = newIndex
                                }
                                dragAmount %= dragThreshold
                            }
                        }
                    ),
                    navController = navController,
                    editWatchlistModel = editWatchlistViewModel
                )
            }
        }
    }
}

fun <T> MutableList<T>.move(from: Int, to: Int) {
    val element = removeAt(from)
    add(to, element)
}

@Composable
fun CompanyProfileItem(
    ticker: String,
    WatchlistModel: WatchListModel,
    modifier: Modifier = Modifier,
    navController: NavController,
    editWatchlistModel: WatchlistViewModel
    ) {

    val companyProfile by WatchlistModel.getProfileStateFlow(ticker).collectAsState()
    val softerGreen = Color(0xFF66BB6A)
    val softerRed = Color(0xFFF34B4B)
    val increaseIconPainter = painterResource(id = R.drawable.icons8_increase_48)
    val decreaseIconPainter = painterResource(id = R.drawable.icons8_decrease_48)
    var offsetX by remember { mutableStateOf(0f) }

    val draggableState = rememberDraggableState { delta ->
        offsetX += delta
        offsetX = offsetX.coerceAtLeast(-390f)
        Log.d("DraggableDemo", "OffsetX after drag: $offsetX")
    }

    LaunchedEffect(ticker) {
        if (ticker.isNotEmpty()) {
            WatchlistModel.watchCompany(ticker)
        }
    }

    Box(
        modifier = modifier
            .draggable(
                state = draggableState,
                orientation = Orientation.Horizontal,
                onDragStopped = {
                    if (offsetX <= -380f) {
                        editWatchlistModel.removeTicker(ticker)
                    }
                    offsetX = 0f
                }
            )
            .fillMaxWidth()
            .height(75.dp)
            .background(Color.White)
    ) {
            Column{
            if (companyProfile != null) {
                Row(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(start = 16.dp, end = 4.dp)
                ) {
                    Column(modifier = Modifier.weight(1f)) {
                        Row(
                            modifier = Modifier
                                .fillMaxWidth()
                                .padding(vertical = 8.dp),
                            horizontalArrangement = Arrangement.SpaceBetween
                        ) {
                            Text(
                                text = ticker,
                                style = TextStyle(
                                    fontWeight = FontWeight.Bold,
                                    fontSize = 20.sp
                                )
                            )
                            Text(
                                text = "$${String.format("%.2f", companyProfile!!.c)}",
                                style = TextStyle(
                                    fontWeight = FontWeight.Bold,
                                    fontSize = 20.sp
                                )
                            )
                        }
                        Row(
                            modifier = Modifier
                                .fillMaxWidth()
                                .padding(vertical = 4.dp),
                            horizontalArrangement = Arrangement.SpaceBetween
                        ) {
                            val (changeColor, iconPainter) = when {
                                companyProfile!!.d > 0 -> softerGreen to increaseIconPainter
                                companyProfile!!.d < 0 -> softerRed to decreaseIconPainter
                                else -> Color.Black to null
                            }

                            Text(
                                text = companyProfile!!.name,
                                color = Color(0xFF888888)
                            )
                            Row(verticalAlignment = Alignment.CenterVertically) {
                                if (iconPainter != null) {
                                    Icon(
                                        painter = iconPainter,
                                        contentDescription = "Change Icon",
                                        tint = changeColor,
                                        modifier = Modifier
                                            .size(20.dp)
                                            .padding(end = 2.dp)
                                    )
                                }
                                Text(
                                    text = "${
                                        String.format(
                                            "%.2f",
                                            companyProfile!!.d
                                        )
                                    }(${String.format("%.2f", companyProfile!!.dp)}%)",
                                    color = changeColor,
                                    style = TextStyle(
                                        fontWeight = FontWeight.Bold,
                                        fontSize = 16.sp
                                    )
                                )
                            }
                        }
                    }
                    Icon(
                        painter = painterResource(id = R.drawable.chevron_right),
                        contentDescription = "Chevron Right",
                        modifier = Modifier
                            .size(24.dp)
                            .align(Alignment.CenterVertically)
                            .clickable {
                                navController.navigate("searchResult/${ticker}") {
                                    popUpTo("mainScreen") { inclusive = true }
                                }
                            }
                    )
                }
                Divider(color = Color.LightGray, thickness = 2.dp)
            }
            }

        if (offsetX < 0) {
            Log.d("DraggableDemo", "Rendering red box with width: ${-offsetX.dp}")
            Box(
                modifier = Modifier
                    .align(Alignment.CenterEnd)
                    .width((-offsetX).dp)
                    .fillMaxHeight()
                    .background(Color.Red)
            )
            Icon(
                painter = painterResource(id = R.drawable.delete),
                contentDescription = "Delete",
                modifier = Modifier.align(Alignment.CenterEnd).padding(end = 8.dp),
                tint = Color.White
            )
        }
    }
}

@Composable
fun SearchToggle(viewModel: CompanyViewModel, navController: NavController) {
    var isSearchVisible by remember { mutableStateOf(false) }
    var searchText by remember { mutableStateOf("") }
    val unfilteredResults by viewModel.searchResults.collectAsState()

    val filteredResults = unfilteredResults?.result?.filter {
                !it.symbol.contains(".") &&
                it.symbol.contains(searchText, ignoreCase = true)
    } ?: emptyList()

    LaunchedEffect(searchText) {
        if (searchText.isNotEmpty()) {
            viewModel.searchCompany(searchText)
        }
    }

    if (isSearchVisible) {
        Column {
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .height(56.dp)
                    .padding(horizontal = 16.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                IconButton(onClick = { isSearchVisible = false }) {
                    Icon(Icons.Filled.ArrowBack, contentDescription = "Back")
                }
                OutlinedTextField(
                    value = searchText,
                    onValueChange = { searchText = it },
                    singleLine = true,
                    placeholder = {
                        Text(
                            text = "Enter a stock symbol",
                            style = TextStyle(fontSize = 20.sp, color = Color.Gray)
                        )
                    },
                    modifier = Modifier.weight(1f),
                    trailingIcon = {
                        if (searchText.isNotEmpty()) {
                            IconButton(onClick = { searchText = "" }) {
                                Icon(Icons.Filled.Clear, contentDescription = "Clear")
                            }
                        }
                    },
                    colors = OutlinedTextFieldDefaults.colors(
                        focusedBorderColor = Color.Transparent,
                        unfocusedBorderColor = Color.Transparent
                    )
                )
            }

            if (searchText.isNotEmpty()) {
                Box(
                    modifier = Modifier
                        .heightIn(max = 400.dp)
                        .fillMaxWidth()
                ) {
                    val scrollState = rememberScrollState()
                    if (filteredResults.isNotEmpty()) {
                        Column(modifier = Modifier.verticalScroll(scrollState)) {
                            filteredResults.forEach { company ->
                                Text(
                                    text = "${company.displaySymbol} | ${company.description}",
                                    modifier = Modifier
                                        .fillMaxWidth()
                                        .padding(16.dp)
                                        .clickable {
                                            navController.navigate("searchResult/${company.displaySymbol}") {
                                                popUpTo("mainScreen") { inclusive = true }
                                            }
                                        }
                                )
                            }
                        }
                    }
                    else {
                        Box(contentAlignment = Alignment.Center, modifier = Modifier.fillMaxWidth()) {
                            CircularProgressIndicator()
                        }
                    }
                }
            }
        }
    }
    else {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .height(56.dp)
                .padding(horizontal = 16.dp, vertical = 4.dp),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically
        ) {
            Text(
                text = "Stocks",
                style = TextStyle(fontSize = 20.sp),
                modifier = Modifier
                    .padding(start = 2.dp, top = 4.dp, bottom = 4.dp, end = 16.dp)
            )
            IconButton(onClick = { isSearchVisible = true }) {
                Icon(Icons.Filled.Search, contentDescription = "Search")
            }
        }
    }
}

@Preview(showBackground = true)
@Composable
fun GreetingPreview() {
    Ass4Theme {

    }
}



