package com.example.ass4.ui.screen


import android.content.Context
import android.util.Log
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Button
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp
import androidx.navigation.NavController
import androidx.compose.foundation.layout.*
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ArrowBack
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.TopAppBar
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.*
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.lifecycle.viewmodel.compose.viewModel
import com.example.ass4.viewmodel.CompanyWatchModel
import androidx.compose.ui.viewinterop.AndroidView
import android.content.Intent
import android.webkit.WebResourceError
import android.webkit.WebResourceRequest
import android.webkit.WebView
import android.webkit.WebViewClient
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.tooling.preview.Preview
import androidx.webkit.WebSettingsCompat
import androidx.webkit.WebViewFeature
import com.example.ass4.api.models.HourlyTradeItem
import com.example.ass4.api.models.TradeItem
import com.example.ass4.viewmodel.CompanyHourlyChartModel
import com.google.gson.Gson
import java.text.SimpleDateFormat
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.text.style.TextDecoration
import androidx.compose.ui.unit.sp
import com.example.ass4.ui.theme.Ass4Theme
import com.example.ass4.api.models.NewsItem
import androidx.compose.foundation.Image
import androidx.compose.foundation.clickable
import androidx.compose.material3.Card
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.text.TextStyle
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.window.Dialog
import coil.compose.rememberImagePainter
import com.example.ass4.api.models.CompanyData
import com.example.ass4.api.models.CompanyEarnings
import com.example.ass4.api.models.CompanyInsider
import com.example.ass4.api.models.CompanyQuote
import com.example.ass4.api.models.RecommendationTrendItem
import android.net.Uri
import android.widget.Toast
import androidx.compose.foundation.lazy.LazyRow
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.window.DialogProperties
import java.util.Date
import java.util.TimeZone
import androidx.compose.ui.platform.LocalUriHandler
import androidx.compose.ui.res.painterResource
import com.example.ass4.R
import androidx.compose.runtime.livedata.observeAsState
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.ColorFilter
import androidx.compose.ui.graphics.painter.Painter
import androidx.lifecycle.Observer
import com.example.ass4.api.AggregatedPortfolioItem
import com.example.ass4.viewmodel.PortfolioCompanyWatchModel
import com.example.ass4.viewmodel.PortfolioInteractModel
import com.example.ass4.viewmodel.WatchlistViewModel



@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun TopNav(
    navController: NavController,
    displaySymbol: String?,
    watchlistViewModel: WatchlistViewModel,
    context: Context = LocalContext.current) {
    val watchlist by watchlistViewModel.watchlist.collectAsState()
    val isSymbolInWatchlist = watchlist.any { it.ticker == displaySymbol }

    Surface(
        modifier = Modifier.fillMaxWidth(),
        shadowElevation = 4.dp,
        shape = RoundedCornerShape(bottomEnd = 4.dp, bottomStart = 4.dp)
    ) {
        TopAppBar(
            title = { Text(text = displaySymbol ?: "Unknown Symbol") },
            navigationIcon = {
                IconButton(onClick = {
                    navController.navigate("mainScreen") {
                        popUpTo("mainScreen") { inclusive = true }
                    }
                }) {
                    Icon(Icons.Filled.ArrowBack, contentDescription = "Back")
                }
            },
            actions = {
                IconButton(onClick = {
                    if (isSymbolInWatchlist) {
                        if (displaySymbol != null) {
                            watchlistViewModel.removeTicker(displaySymbol)
                            Toast.makeText(context, "$displaySymbol is removed from favorites", Toast.LENGTH_SHORT).show()
                        }
                    } else {
                        if (displaySymbol != null) {
                            watchlistViewModel.addTicker(displaySymbol)
                            Toast.makeText(context, "$displaySymbol is added to favorites", Toast.LENGTH_SHORT).show()
                        }
                    }
                }) {
                    val imagePainter: Painter = painterResource(id = if (isSymbolInWatchlist) R.drawable.icons8_starfilled_50 else R.drawable.icons8_star_50)
                    Image(
                        painter = imagePainter,
                        contentDescription = if (isSymbolInWatchlist) "Filled Star" else "Empty Star"
                    )
                }
            }
        )
    }
}

@Composable
fun BasicInfo(viewModel: CompanyWatchModel, displaySymbol: String?) {
    val companyInfo = viewModel.watchResults.collectAsState().value
    val lastLoadedSymbol = remember { mutableStateOf<String?>(null) }

    LaunchedEffect(displaySymbol) {
        if (displaySymbol != null && displaySymbol != lastLoadedSymbol.value) {
            viewModel.watchCompany(displaySymbol)
            lastLoadedSymbol.value = displaySymbol
        }
    }

    val softerGreen = remember { Color(0xFF66BB6A) }
    val softerRed = remember { Color(0xFFF34B4B) }
    val increaseIconPainter = painterResource(id = R.drawable.icons8_increase_48)
    val decreaseIconPainter = painterResource(id = R.drawable.icons8_decrease_48)

    Column(modifier = Modifier.padding(bottom = 10.dp)) {
        if (companyInfo != null) {
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(horizontal = 16.dp, vertical = 8.dp),
                horizontalArrangement = Arrangement.SpaceBetween
            ) {
                Text(
                    text = companyInfo.ticker,
                    style = TextStyle(
                        fontWeight = FontWeight.Bold,
                        fontSize = 20.sp
                    )
                )
                Text(
                    text = "$${companyInfo.c}",
                    style = TextStyle(
                        fontWeight = FontWeight.Bold,
                        fontSize = 20.sp
                    )
                )
            }

            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(horizontal = 16.dp, vertical = 4.dp),
                horizontalArrangement = Arrangement.SpaceBetween
            ) {
                Text(
                    text = companyInfo.name
                )
                val (changeColor, iconPainter) = when {
                    companyInfo.d > 0 -> softerGreen to increaseIconPainter
                    companyInfo.d < 0 -> softerRed to decreaseIconPainter
                    else -> Color.Black to null
                }
                Row(verticalAlignment = Alignment.CenterVertically) {
                    if (iconPainter != null) {
                        Icon(
                            painter = iconPainter,
                            contentDescription = "Change Icon",
                            tint = changeColor,
                            modifier = Modifier.size(20.dp).padding(end = 2.dp)
                        )
                    }
                    Text(
                        text = "\$${String.format("%.2f", companyInfo.d)} (${String.format("%.2f", companyInfo.dp)}%)",
                        color = changeColor,
                        style = TextStyle(
                            fontWeight = FontWeight.Bold,
                            fontSize = 16.sp
                        )
                    )
                }
            }
        } else {
            Text("Loading company details...")
        }
    }
}

@Composable
fun ImageSelectionRow(selectedImage: String, onImageSelected: (String) -> Unit) {
    Surface {
        Row(
            verticalAlignment = Alignment.CenterVertically,
            modifier = Modifier
                .fillMaxWidth()
                .padding(vertical = 8.dp)
        ) {
            Box(
                contentAlignment = Alignment.Center,
                modifier = Modifier
                    .weight(1f)
                    .fillMaxHeight()
                    .clickable { onImageSelected(if (selectedImage == "chart") "" else "chart") }
            ) {
                Image(
                    painter = painterResource(id = R.drawable.icons8_combo_chart_24),
                    contentDescription = "Chart Icon",
                    modifier = Modifier
                        .size(50.dp)
                        .padding(10.dp),
                    colorFilter = if (selectedImage == "chart") ColorFilter.tint(Color.Blue) else null
                )
                if (selectedImage == "chart") {
                    Box(
                        modifier = Modifier
                            .align(Alignment.BottomCenter)
                            .height(2.dp)
                            .fillMaxWidth()
                            .background(Color.Blue)
                    )
                }
            }
            Box(
                contentAlignment = Alignment.Center,
                modifier = Modifier
                    .weight(1f)
                    .fillMaxHeight()
                    .clickable { onImageSelected(if (selectedImage == "clock") "" else "clock") }
            ) {
                Image(
                    painter = painterResource(id = R.drawable.icons8_clock_48),
                    contentDescription = "Clock Icon",
                    modifier = Modifier
                        .size(50.dp)
                        .padding(10.dp),
                    colorFilter = if (selectedImage == "clock") ColorFilter.tint(Color.Blue) else null
                )
                if (selectedImage == "clock") {
                    Box(
                        modifier = Modifier
                            .align(Alignment.BottomCenter)
                            .height(2.dp)
                            .fillMaxWidth()
                            .background(Color.Blue)
                    )
                }
            }
        }
    }
}

fun formatHourlyTradeData(hourlyTradeSet: List<HourlyTradeItem>?): String {
    if (hourlyTradeSet == null || hourlyTradeSet.isEmpty()) {

        return "[]"
    }
    return hourlyTradeSet.joinToString(
        separator = ",",
        prefix = "[",
        postfix = "]"
    ) { trade ->
        "[${trade.Date}, ${trade.StockPrice}]"
    }
}

@Composable
fun HourlyChartInfo(
    viewModel: CompanyHourlyChartModel,
    displaySymbol: String?,
    visible: Boolean,
    getChangeViewModel: CompanyWatchModel
) {
    if (visible && displaySymbol != null) {
        LaunchedEffect(displaySymbol) {
            viewModel.getCompany(displaySymbol)
            getChangeViewModel.watchCompany(displaySymbol)
        }
        val companyInfo = getChangeViewModel.watchResults.collectAsState().value
        val responseData by viewModel.hourlyResults.collectAsState()


        var lastNonEmptyFormattedData by remember { mutableStateOf("") }
        val currentFormattedData = formatHourlyTradeData(responseData?.hourlyTradeSet)
        if (currentFormattedData != "[]") {
            lastNonEmptyFormattedData = currentFormattedData
        }
        var seriesColor = if ((companyInfo?.d ?: 0.0) < 0) "#ba1414" else "#208a3c"
        if (lastNonEmptyFormattedData.isNotEmpty()) {
            HighChartsWebView(lastNonEmptyFormattedData, displaySymbol, seriesColor)
        }
        else {
            Box(modifier = Modifier.fillMaxSize()) {
                CircularProgressIndicator(modifier = Modifier.align(Alignment.Center))
            }
        }
    }
    else {
        Box(modifier = Modifier.fillMaxSize()) {
            CircularProgressIndicator(modifier = Modifier.align(Alignment.Center))
        }
    }
}

@Composable
fun YearlyChartInfo(viewModel: CompanyHourlyChartModel, displaySymbol: String?, visible: Boolean) {
    if (visible) {
        val stableDisplaySymbol = remember { displaySymbol }
        val responseData by viewModel.hourlyResults.collectAsState()

        LaunchedEffect(stableDisplaySymbol) {
            if (stableDisplaySymbol != null) {
                viewModel.getCompany(stableDisplaySymbol)
            }
        }


        var lastNonEmptyTradeSet by remember { mutableStateOf<Map<String, Any>>(emptyMap()) }
        val currentTradeSet =
            stableDisplaySymbol?.let { prepareYearlyChartData(responseData?.tradeSet, it) }
        currentTradeSet?.let {
            val ohlcList = it["ohlc"] as? List<*> ?: emptyList<Any>()
            val volumeList = it["volume"] as? List<*> ?: emptyList<Any>()
            if (ohlcList.isNotEmpty() && volumeList.isNotEmpty()) {
                lastNonEmptyTradeSet = it
            }
        }

        if (stableDisplaySymbol != null && lastNonEmptyTradeSet != null && hasValidChartData(
                lastNonEmptyTradeSet
            )
        ) {
            YearlyChartsWebView(lastNonEmptyTradeSet!!)
        }
        else if (stableDisplaySymbol == null) {
            Box(modifier = Modifier.fillMaxSize()) {
                CircularProgressIndicator(modifier = Modifier.align(Alignment.Center))
            }
        }
        else {
            Box(modifier = Modifier.fillMaxSize()) {
                CircularProgressIndicator(modifier = Modifier.align(Alignment.Center))
            }
        }
    }
}

private fun hasValidChartData(data: Map<String, Any>?): Boolean {
    return data?.let {
        val ohlcList = it["ohlc"] as? List<*> ?: emptyList<Any>()
        val volumeList = it["volume"] as? List<*> ?: emptyList<Any>()
        ohlcList.isNotEmpty() && volumeList.isNotEmpty()
    } ?: false
}

fun prepareYearlyChartData(tradeSet: List<TradeItem>?, ticker: String): Map<String, Any> {
    val ohlc = mutableListOf<List<Any>>()
    val volume = mutableListOf<List<Any>>()
    val offset = 8 * 3600 * 1000

    if (tradeSet.isNullOrEmpty()) {
        Log.d("ChartData", "Trade set is null or empty.")
    }
    else {
        tradeSet.forEach { tradeItem ->
            if (tradeItem.Date != null && tradeItem.Open != null && tradeItem.High != null &&
                tradeItem.Low != null && tradeItem.Close != null && tradeItem.Volume != null) {
                ohlc.add(listOf(tradeItem.Date - offset, tradeItem.Open, tradeItem.High, tradeItem.Low, tradeItem.Close))
                volume.add(listOf(tradeItem.Date - offset, tradeItem.Volume))
            }
            else {
                Log.d("ChartData", "Incomplete trade item found, skipping...")
            }
        }
    }

    return mapOf(
        "ohlc" to ohlc,
        "volume" to volume,
        "name" to ticker
    )
}


@Composable
fun YearlyChartsWebView(data: Map<String, Any>) {
    val ohlcData = data["ohlc"] ?: "OHLC data not available"
    val volumeData = data["volume"] ?: "Volume data not available"
    val nameData = data["name"] ?: "Name not available"

    AndroidView(
        factory = { context ->
            WebView(context).apply {
                webViewClient = object : WebViewClient() {
                    override fun onReceivedError(view: WebView?, request: WebResourceRequest?, error: WebResourceError?) {
                        super.onReceivedError(view, request, error)
                        Log.e("WebViewError", "Error loading page: ${error?.description}")
                    }
                }
                settings.javaScriptEnabled = true


                val htmlContent = context.assets.open("test_chart.html").bufferedReader().use { it.readText() }
                    .replace("%%OHLC_JSON%%", gson.toJson(data["ohlc"]))
                    .replace("%%VOLUME_JSON%%", gson.toJson(data["volume"]))
                    .replace("%%NAME_JSON%%", gson.toJson(data["name"]))



                loadDataWithBaseURL(
                    null,
                    htmlContent,
                    "text/html",
                    "UTF-8",
                    null
                )
            }
        },
        modifier = Modifier.fillMaxSize()
    )
}

@Composable
fun RecommendationChartInfo(viewModel: CompanyHourlyChartModel, displaySymbol: String?) {

    val stableDisplaySymbol = remember { displaySymbol }
    val responseData by viewModel.hourlyResults.collectAsState()

    LaunchedEffect(stableDisplaySymbol) {
        if (stableDisplaySymbol != null) {
            viewModel.getCompany(stableDisplaySymbol)
        }
    }

    var lastNonEmptyTrend by remember { mutableStateOf<List<TrendSeries>>(emptyList()) }
    val currentTrend = createTrendSeries(responseData?.recommendationTrend)
    var lastNonEmptyCategory by remember { mutableStateOf<List<String>>(emptyList()) }

    if (currentTrend.isNotEmpty()) {
        lastNonEmptyTrend = currentTrend
        lastNonEmptyCategory = prepareChartData(responseData?.recommendationTrend)
    }

    if (stableDisplaySymbol != null && lastNonEmptyTrend.isNotEmpty()) {
        TrendChartsWebView(lastNonEmptyCategory,lastNonEmptyTrend)
    } else {
        Box(modifier = Modifier.fillMaxSize()) {
            CircularProgressIndicator(modifier = Modifier.align(Alignment.Center))
        }
    }
}

data class TrendDataPoint(
    val y: Int,
    val period: String
)

data class TrendSeries(
    val name: String,
    val data: List<TrendDataPoint>,
    val color: String
)

val gson = Gson()

// from ChatGPT
fun serializeTrendSeries(series: List<TrendSeries>): String {
    return gson.toJson(series)
}

// from ChatGPT
fun serializeCategories(categories: List<String>): String {
    return gson.toJson(categories)
}

fun createTrendSeries(RecTrend: List<RecommendationTrendItem>?): List<TrendSeries> {
    if (RecTrend == null || RecTrend.isEmpty()) return emptyList()
    return listOf(
        TrendSeries(
            name = "Strong Buy",
            data = RecTrend.map { TrendDataPoint(y = it.strongBuy, period = it.period) },
            color = "#206b00"
        ),
        TrendSeries(
            name = "Buy",
            data = RecTrend.map { TrendDataPoint(y = it.Buy, period = it.period) },
            color = "#3fc904"
        ),
        TrendSeries(
            name = "Hold",
            data = RecTrend.map { TrendDataPoint(y = it.Hold, period = it.period) },
            color = "#a87402"
        ),
        TrendSeries(
            name = "Sell",
            data = RecTrend.map { TrendDataPoint(y = it.sell, period = it.period) },
            color = "#eb0000"
        ),
        TrendSeries(
            name = "Strong Sell",
            data = RecTrend.map { TrendDataPoint(y = it.strongSell, period = it.period) },
            color = "#4d0202"
        )
    )
}

fun prepareChartData(trendData: List<RecommendationTrendItem>?): List<String> {
    if (trendData == null || trendData.isEmpty()) return emptyList()
    return trendData.map { it.period.slice(0..6) }
}

@Composable
fun TrendChartsWebView(categories: List<String>, seriesData: List<TrendSeries>) {
    val jsonCategories = serializeCategories(categories)
    val jsonSeriesData = serializeTrendSeries(seriesData)
    AndroidView(factory = { context ->
        WebView(context).apply {
            webViewClient = object : WebViewClient() {
                override fun onReceivedError(view: WebView?, request: WebResourceRequest?, error: WebResourceError?) {
                    super.onReceivedError(view, request, error)
                    Log.e("WebViewError", "Error loading page ${error?.description}")
                }
            }
            settings.javaScriptEnabled = true

            val htmlContent = context.assets.open("trend_chart.html")
                .bufferedReader().use { it.readText() }
                .replace("%%CATEGORIES_JSON%%", jsonCategories)
                .replace("%%SERIES_JSON%%", jsonSeriesData)

            loadDataWithBaseURL(
                null,
                htmlContent,
                "text/html",
                "UTF-8",
                null
            )
        }
    }, modifier = Modifier.fillMaxSize())
}

@Composable
fun HighChartsWebView(chartData: String, Ticker: String, seriesColor: String) {

    AndroidView(
        factory = { context ->
            WebView(context).apply {
                webViewClient = WebViewClient()
                settings.apply {
                    javaScriptEnabled = true
                    if (WebViewFeature.isFeatureSupported(WebViewFeature.FORCE_DARK)) {
                        WebSettingsCompat.setForceDark(this, WebSettingsCompat.FORCE_DARK_AUTO)
                    }
                }
                loadDataWithBaseURL(
                    null,
                    getHtmlData(chartData, Ticker, seriesColor),
                    "text/html",
                    "UTF-8",
                    null
                )
            }
        },
        modifier = Modifier.fillMaxSize()
    )
}

private fun getHtmlData(chartData: String, Ticker: String, seriesColor: String): String {
    Log.d("Hourly", "draw: $chartData")
    Log.d("Hourly", "color: $seriesColor")
    return """
        <html>
        <head>
            <script src="https://code.highcharts.com/highcharts.js"></script>
        </head>
        <body>
            <div id="container" style="width:100%; height:100%;"></div>
            <script>
                Highcharts.chart('container', {
                    chart: {
                        type: 'line',
                        backgroundColor: '#f0f0f0',
                        scrollablePlotArea: {
                            minWidth: 500,
                            scrollPositionX: 1
                        }
                    },
                    title: {
                        text: '${Ticker} Hourly Price Variation',
                        style: {
                            color: '#878787'
                        }
                    },
                    xAxis: {
                        type: 'datetime',
                        dateTimeLabelFormats: {
                            hour: '%H:%M',
                        },
                        title: {
                            text: ''
                        },
                        scrollbar: {
                            enabled: true,
                        },
                        labels: {
                            overflow: 'justify'
                        }
                    },
                    yAxis: {
                        title: {
                            text: ''
                        },
                        opposite: true,
                    },
                    series: [{
                        name: 'Stock Price',
                        data: $chartData,
                        color: '$seriesColor',
                    }],
                    plotOptions: {
                        line: {
                            dataLabels: {
                                enabled: false
                            },
                            marker: {
                                enabled: false
                            },
                            enableMouseTracking: false
                        }
                    },
                    legend: {
                        enabled: false
                    }
                });
            </script>
        </body>
        </html>
    """
}

@Composable
fun EarningChartInfo(viewModel: CompanyHourlyChartModel, displaySymbol: String?) {
    val stableDisplaySymbol = remember { displaySymbol }
    val responseData by viewModel.hourlyResults.collectAsState()

    LaunchedEffect(stableDisplaySymbol) {
        if (stableDisplaySymbol != null) {
            viewModel.getCompany(stableDisplaySymbol)
        }
    }

    var lastNonEarnings by remember { mutableStateOf<List<EarningSeries>>(emptyList()) }
    val currentEarnings = createEarningSeries(responseData?.companyEarnings)
    var lastNonEmptyCategory by remember { mutableStateOf<List<String>>(emptyList()) }
    if (currentEarnings.isNotEmpty()) {
        lastNonEarnings = currentEarnings
        lastNonEmptyCategory = prepareEarningChartData(responseData?.companyEarnings)
    }

    if (stableDisplaySymbol != null && lastNonEarnings.isNotEmpty()) {
        EarningChartsWebView(lastNonEmptyCategory,lastNonEarnings)
    } else {
        Box(modifier = Modifier.fillMaxSize()) {
            CircularProgressIndicator(modifier = Modifier.align(Alignment.Center))
        }
    }
}

fun prepareEarningChartData(earningData: List<CompanyEarnings>?): List<String> {
    if (earningData == null || earningData.isEmpty()) return emptyList()
    return earningData.map { earning ->
        "${earning.period}<br> Surprise: ${earning.surprise}"
    }
}

data class EarningsDataPoint(
    val y: Double
)

data class EarningSeries(
    val name: String,
    val data: List<EarningsDataPoint>,
)

fun createEarningSeries(earnings: List<CompanyEarnings>?): List<EarningSeries> {
    if (earnings == null || earnings.isEmpty()) return emptyList()
    return listOf(
        EarningSeries(
            name = "Actual",
            data = earnings.map { EarningsDataPoint(y = it.actual) }
        ),
        EarningSeries(
            name = "Estimate",
            data = earnings.map { EarningsDataPoint(y = it.estimate) }
        )
    )
}

// from ChatGPT
fun serializeEarningSeries(series: List<EarningSeries>): String {
    return gson.toJson(series)
}

// from ChatGPT
fun serializeEarningCategories(categories: List<String>): String {
    return gson.toJson(categories)
}

@Composable
fun EarningChartsWebView(categories: List<String>, seriesData: List<EarningSeries>) {
    val jsonCategories = serializeEarningCategories(categories)
    val jsonSeriesData = serializeEarningSeries(seriesData)
    AndroidView(factory = { context ->
        WebView(context).apply {
            webViewClient = object : WebViewClient() {
                override fun onReceivedError(view: WebView?, request: WebResourceRequest?, error: WebResourceError?) {
                    super.onReceivedError(view, request, error)
                    Log.e("WebViewError", "Error loading page ${error?.description}")
                }
            }
            settings.javaScriptEnabled = true
            val htmlContent = context.assets.open("mspr_chart.html")
                .bufferedReader().use { it.readText() }
                .replace("%%CATEGORIES_JSON%%", jsonCategories)
                .replace("%%SERIES_JSON%%", jsonSeriesData)

            loadDataWithBaseURL(
                null,
                htmlContent,
                "text/html",
                "UTF-8",
                null
            )
        }
    }, modifier = Modifier.fillMaxSize())
}

@Composable
fun PortfolioTradeBox(
    viewModel: PortfolioInteractModel,
    displaySymbol: String?,
    watchModel: PortfolioCompanyWatchModel,
    portfolioViewModel: PortfolioInteractModel
) {
    val aggregatedPortfolioItems by viewModel.aggregatedPortfolioDetails.observeAsState(initial = emptyList())
    val refreshTrigger by viewModel.refreshTrigger.observeAsState()

    LaunchedEffect(refreshTrigger) {
        viewModel.fetchTickers()
    }

    Surface(
        modifier = Modifier
            .fillMaxWidth()
            .padding(vertical = 8.dp)
    ) {
        Text(
            modifier = Modifier
                .padding(4.dp),
            text = "Portfolio",
            style = MaterialTheme.typography.titleLarge.copy(fontWeight = FontWeight.Bold),
            fontSize = 20.sp,
        )
        Column(modifier = Modifier.padding(16.dp)) {
            Spacer(modifier = Modifier.height(16.dp))
            Row(
                modifier = Modifier.fillMaxWidth(),
                verticalAlignment = Alignment.CenterVertically
            ) {
                if (displaySymbol != null) {
                    AggregatedPortfolioList(aggregatedPortfolioItems,displaySymbol, watchModel)
                    Spacer(modifier = Modifier.weight(1f))
                    TradeButton(aggregatedPortfolioItems, displaySymbol, watchModel
                        ,portfolioViewModel)
                }
            }
        }
    }
}

@Composable
fun AggregatedPortfolioList(portfolioItems: List<AggregatedPortfolioItem>, displaySymbol: String, watchModel: PortfolioCompanyWatchModel) {
    val filteredItems = portfolioItems.filter { item ->
        item.ticker == displaySymbol
    }

    if (filteredItems.isEmpty()) {
        DummyPortfolioDetails()
    } else {
        filteredItems.forEach { item ->
            PortfolioDetails(item, watchModel)
        }
    }
}

@Composable
fun DummyPortfolioDetails() {

    Column {
        Spacer(modifier = Modifier.height(10.dp))
        Text("Shares Owned:", style = MaterialTheme.typography.bodyLarge)
        Spacer(modifier = Modifier.height(16.dp))
        Text("Avg. Cost/Share:", style = MaterialTheme.typography.bodyLarge)
        Spacer(modifier = Modifier.height(16.dp))
        Text("Total Cost:", style = MaterialTheme.typography.bodyLarge)
        Spacer(modifier = Modifier.height(16.dp))
        Text("Change:", style = MaterialTheme.typography.bodyLarge)
        Spacer(modifier = Modifier.height(16.dp))
        Text("Market Value:", style = MaterialTheme.typography.bodyLarge)
    }

    Column(modifier = Modifier.padding(horizontal = 20.dp)) {
        Spacer(modifier = Modifier.height(10.dp))
        Text(
            text = "0",
            style = MaterialTheme.typography.bodyLarge
        )
        Spacer(modifier = Modifier.height(16.dp))
        Text(
            text = "$0.00",
            style = MaterialTheme.typography.bodyLarge
        )
        Spacer(modifier = Modifier.height(16.dp))
        Text(
            text = "$0.00",
            style = MaterialTheme.typography.bodyLarge
        )
        Spacer(modifier = Modifier.height(16.dp))
        Text(
            text = "$0.00",
            style = MaterialTheme.typography.bodyLarge
        )
        Spacer(modifier = Modifier.height(16.dp))
        Text(
            text = "$0.00",
            style = MaterialTheme.typography.bodyLarge
        )
    }
}

@Composable
fun PortfolioDetails(item: AggregatedPortfolioItem, watchModel: PortfolioCompanyWatchModel) {
    val companyProfile by watchModel.getProfileStateFlow(item.ticker).collectAsState()
    LaunchedEffect(item.ticker) {
        watchModel.PortfolioWatchCompany(item.ticker)
    }

    val green = Color(0xFF66BB6A)
    val red = Color(0xFFF34B4B)
    val black = Color.Black

    Column {
        Spacer(modifier = Modifier.height(10.dp))
        Text("Shares Owned:", style = MaterialTheme.typography.bodyLarge)
        Spacer(modifier = Modifier.height(16.dp))
        Text("Avg. Cost/Share:", style = MaterialTheme.typography.bodyLarge)
        Spacer(modifier = Modifier.height(16.dp))
        Text("Total Cost:", style = MaterialTheme.typography.bodyLarge)
        Spacer(modifier = Modifier.height(16.dp))
        Text("Change:", style = MaterialTheme.typography.bodyLarge)
        Spacer(modifier = Modifier.height(16.dp))
        Text("Market Value:", style = MaterialTheme.typography.bodyLarge)
    }

    Column(modifier = Modifier.padding(horizontal = 20.dp)) {
        Spacer(modifier = Modifier.height(10.dp))
        Text(
            text = "${item.totalShares ?: 0}",
            style = MaterialTheme.typography.bodyLarge
        )
        Spacer(modifier = Modifier.height(16.dp))
        Text(
            text = "$${String.format("%.2f", item.averageCost ?: 0.0)}",
            style = MaterialTheme.typography.bodyLarge
        )
        Spacer(modifier = Modifier.height(16.dp))
        Text(
            text = "$${String.format("%.2f", item.totalCost ?: 0.0)}",
            style = MaterialTheme.typography.bodyLarge
        )
        Spacer(modifier = Modifier.height(16.dp))
        Text(
            text = companyProfile?.let {                         // from ChatGPT
                val netValue = it.c - (item.averageCost ?: 0.0)                 // from ChatGPT
                val color = when {                                              // from ChatGPT
                    netValue > 0 -> green                                       // from ChatGPT
                    netValue < 0 -> red                                         // from ChatGPT
                    else -> black                                               // from ChatGPT
                }
                "\$${String.format("%.2f", netValue)}"
            } ?: "\$--",
            style = MaterialTheme.typography.bodyLarge.copy(color = companyProfile?.let {
                val netValue = it.c - (item.averageCost ?: 0.0)
                when {
                    netValue > 0 -> green
                    netValue < 0 -> red
                    else -> black
                }
            } ?: black)
        )

        Spacer(modifier = Modifier.height(16.dp))
        Text(
            text = companyProfile?.let {                            // from ChatGPT
                val netValue = it.c - (item.averageCost ?: 0.0)                    // from ChatGPT
                val price = it.c                                                   // from ChatGPT
                val totalValue = price * (item.totalShares ?: 0)                   // from ChatGPT
                val color = when {                                                 // from ChatGPT
                    netValue > 0 -> green                                          // from ChatGPT
                    netValue < 0 -> red                                            // from ChatGPT
                    else -> black
                }
                "\$${String.format("%.2f", totalValue)}"
            } ?: "\$--",
            style = MaterialTheme.typography.bodyLarge.copy(color = companyProfile?.let {
                val netValue = it.c - (item.averageCost ?: 0.0)
                val price = it.c
                when {
                    netValue > 0 -> green
                    netValue < 0 -> red
                    else -> black
                }
            } ?: black)
        )
    }
}

@Composable
fun TradeButton(portfolioItems: List<AggregatedPortfolioItem>, displaySymbol: String, watchModel: PortfolioCompanyWatchModel
                ,portfolioViewModel: PortfolioInteractModel) {
    var filteredItems = portfolioItems.filter { item ->
        item.ticker == displaySymbol
    }

    if (filteredItems.isEmpty()) {
        filteredItems = listOf(
            AggregatedPortfolioItem(
                ticker = displaySymbol,
                totalShares = 0,
                averageCost = 0.0,
                totalCost = 0.0
            )
        )
    }

    val customGreen = Color(0xFF30B635)
    val greyText = Color(0xFFFFFFFF)
    var showDialog by remember { mutableStateOf(false) }

    Button(
        onClick = { showDialog = true },
        colors = ButtonDefaults.buttonColors(
            containerColor = customGreen,
            contentColor = greyText,
            disabledContainerColor = Color.Gray,
            disabledContentColor = Color.LightGray
        ),
        shape = RoundedCornerShape(8.dp)
    ) {
        Text("Trade")
    }
    if (showDialog) {
        filteredItems.forEach { item ->
            TradeDialog(item, onDismiss = { showDialog = false }, watchModel, portfolioViewModel)
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun TradeDialog(item: AggregatedPortfolioItem, onDismiss: () -> Unit,watchModel: PortfolioCompanyWatchModel
                ,portfolioViewModel: PortfolioInteractModel, context: Context = LocalContext.current) {
    val companyProfile by watchModel.getProfileStateFlow(item.ticker).collectAsState()
    LaunchedEffect(item.ticker) {
        watchModel.PortfolioWatchCompany(item.ticker)
    }
    var showCongratulationsDialog by remember { mutableStateOf(false) }
    var showDialog by remember { mutableStateOf(true) }
    var transactionType by remember { mutableStateOf("") }
    var sharesNumber by remember { mutableStateOf(0) }
    var transactionStatus by remember { mutableStateOf("") }
    val cashBalance by portfolioViewModel.cashBalance.collectAsState(initial = null)
    val customGreen = Color(0xFF30B635)
    val greyText = Color(0xFFFFFFFF)
    var textFieldValue by remember { mutableStateOf("") }
    if (showDialog) {
        Dialog(onDismissRequest = { showDialog = false; onDismiss() }) {

            Surface(
                shape = RoundedCornerShape(12.dp),
                modifier = Modifier.fillMaxWidth()
            ) {
                Column(
                    modifier = Modifier.padding(1.dp).fillMaxWidth(),
                    horizontalAlignment = Alignment.CenterHorizontally
                ) {
                    Spacer(modifier = Modifier.height(30.dp))
                    Text(
                        text = "Trade ${companyProfile?.name} shares",
                        style = MaterialTheme.typography.titleLarge.copy(fontWeight = FontWeight.Bold),
                        fontSize = 20.sp
                    )
                    Spacer(modifier = Modifier.height(30.dp))
                    Row(
                        verticalAlignment = Alignment.CenterVertically,
                        horizontalArrangement = Arrangement.SpaceAround,
                        modifier = Modifier.fillMaxWidth()
                            .padding(10.dp)
                    ) {
                        TextField(
                            value = textFieldValue,
                            onValueChange = { input ->
                                textFieldValue = input
                                if (input.isNotEmpty() && !input.all { char -> char.isDigit() || (char == '-' && input.indexOf(char) == 0 && input.length == 1) }) {
                                    Toast.makeText(context, "Please enter a valid amount", Toast.LENGTH_SHORT).show()
                                }
                            },
                            placeholder = { Text("0") },
                            singleLine = true,
                            modifier = Modifier.width(200.dp)
                                .height(50.dp),
                            colors = TextFieldDefaults.textFieldColors(

                                unfocusedIndicatorColor = Color.Transparent,
                                focusedIndicatorColor = Color.Blue,
                                cursorColor = Color.Black
                            )
                        )
                        Spacer(modifier = Modifier.width(8.dp))
                        Text("shares")
                    }
                    Row(
                        modifier = Modifier
                            .fillMaxWidth()
                            .padding(10.dp),
                        horizontalArrangement = Arrangement.End
                    ) {
                        val sharePrice = companyProfile?.c ?: 0.0
                        val formattedSharePrice = String.format("%.2f", sharePrice)
                        val sharesToTrade =
                            if (textFieldValue.isNotBlank() && textFieldValue.all { it.isDigit() }) textFieldValue else "0"
                        val totalPrice = sharesToTrade.toDouble() * sharePrice
                        val formattedTotalPrice = String.format("%.2f", totalPrice)
                        Text(
                            text = "$sharesToTrade * \$$formattedSharePrice/share = \$$formattedTotalPrice",
                            style = TextStyle(textAlign = TextAlign.Right)
                        )
                    }
                    Row(
                        modifier = Modifier
                            .fillMaxWidth()
                            .padding(6.dp),
                        horizontalArrangement = Arrangement.Center
                    ) {
                        Text(
                            text = if (cashBalance != null)
                                "${String.format("%.2f", cashBalance)} to buy ${item.ticker}"
                            else
                                "Loading balance..."
                        )
                    }

                    Spacer(modifier = Modifier.height(10.dp))
                    Row(
                        modifier = Modifier.fillMaxWidth(),
                        horizontalArrangement = Arrangement.Center
                    )
                    {
                        Button(
                            onClick = {
                                val numberOfShares = textFieldValue.toIntOrNull() ?: 0
                                if (numberOfShares <= 0) {
                                    Toast.makeText(
                                        context,
                                        "Cannot buy non-positive shares",
                                        Toast.LENGTH_SHORT
                                    ).show()
                                }
                                else {
                                    companyProfile?.let { profile ->
                                        val totalCost = numberOfShares * (profile.c ?: 0.0)
                                        if (cashBalance != null && cashBalance!! >= totalCost) {
                                            portfolioViewModel.purchaseShares(
                                                item.ticker,
                                                profile.name,
                                                numberOfShares,
                                                profile.c ?: 0.0
                                            ).observeForever { result ->
                                                if (result.startsWith("Purchase successful")) {
                                                    showDialog = false
                                                    showCongratulationsDialog = true
                                                    portfolioViewModel.triggerRefresh()
                                                    portfolioViewModel.updateBalance(cashBalance!! -totalCost)
                                                    transactionType = "bought"
                                                    sharesNumber = numberOfShares
                                                }
                                            }
                                        }
                                        else {
                                            Toast.makeText(
                                                context,
                                                "Not enough money to buy.",
                                                Toast.LENGTH_LONG
                                            ).show()
                                        }
                                    }
                                }
                            },
                            colors = ButtonDefaults.buttonColors(
                                containerColor = customGreen,
                                contentColor = greyText,
                                disabledContainerColor = Color.Gray,
                                disabledContentColor = Color.LightGray
                            ),
                            shape = RoundedCornerShape(4.dp)
                        ) {
                            Text("Buy")
                        }
                        Spacer(modifier = Modifier.width(50.dp))
                        Button(
                            onClick = {
                                val numberOfShares = textFieldValue.toIntOrNull() ?: 0
                                if (numberOfShares <= 0) {
                                    Toast.makeText(
                                        context,
                                        "Cannot sell non-positive shares",
                                        Toast.LENGTH_SHORT
                                    ).show()
                                }
                                else {
                                    if (numberOfShares > (item.totalShares ?: 0)) {
                                        Toast.makeText(
                                            context,
                                            "Not enough shares to sell.",
                                            Toast.LENGTH_SHORT
                                        ).show()
                                    }
                                    else {
                                        companyProfile?.let { profile ->
                                            portfolioViewModel.sellShares(
                                                item.ticker,
                                                profile.c,
                                                numberOfShares
                                            ).observeForever { result ->
                                                if (result.startsWith("Sale successful")) {
                                                    showDialog = false
                                                    showCongratulationsDialog = true
                                                    transactionType = "sold"
                                                    sharesNumber = numberOfShares
                                                    portfolioViewModel.triggerRefresh()
                                                    portfolioViewModel.updateBalance(cashBalance!! + (profile.c * numberOfShares))
                                                }
                                            }
                                        }
                                    }
                                }
                            },
                            colors = ButtonDefaults.buttonColors(
                                containerColor = customGreen,
                                contentColor = greyText,
                                disabledContainerColor = Color.Gray,
                                disabledContentColor = Color.LightGray
                            ),
                            shape = RoundedCornerShape(4.dp)
                        ) {
                            Text("Sell")
                        }
                    }
                }
            }
        }
    }

    val customWhite = Color(0xFFFFFFFF)
    val greenText = Color(0xFF1EA823)

    if (showCongratulationsDialog) {
        Dialog(onDismissRequest = {
            showCongratulationsDialog = false
            showDialog = false
            onDismiss()
        }) {
            Surface(
                shape = RoundedCornerShape(16.dp),
                modifier = Modifier.padding(1.dp),
                color = Color(0xFF1EA823)
            ) {
                Column(
                    modifier = Modifier.padding(16.dp),
                    horizontalAlignment = Alignment.CenterHorizontally,
                    verticalArrangement = Arrangement.Center
                ) {
                    Spacer(modifier = Modifier.height(50.dp))
                    Text(
                        "Congratulations!",
                        style = TextStyle(
                            color = Color.White,
                            fontWeight = FontWeight.Bold,
                            fontSize = 20.sp
                        )
                    )
                    Spacer(modifier = Modifier.height(20.dp))
                    Text(
                        "You have successfully $transactionType $sharesNumber",
                        style = TextStyle(
                            color = Color.White,
                            fontSize = 16.sp
                        )
                    )
                    Text(
                        "shares of ${item.ticker}",
                        style = TextStyle(
                            color = Color.White,
                            fontSize = 16.sp
                        )
                    )
                    Spacer(modifier = Modifier.height(30.dp))
                    Button(onClick = {
                        showCongratulationsDialog = false
                        showDialog = false
                    },
                        modifier = Modifier.padding(10.dp).fillMaxWidth(),
                        colors = ButtonDefaults.buttonColors(
                            containerColor = customWhite,
                            contentColor = greenText,
                            disabledContainerColor = Color.Gray,
                            disabledContentColor = Color.LightGray
                        ),
                        shape = RoundedCornerShape(8.dp)) {
                        Text("Done")
                    }
                }
            }
        }
    }
}

@Composable
fun StatsBoxInfo(viewModel: CompanyHourlyChartModel, displaySymbol: String?) {

    val stableDisplaySymbol = remember { displaySymbol }
    val responseData by viewModel.hourlyResults.collectAsState()

    LaunchedEffect(stableDisplaySymbol) {
        if (stableDisplaySymbol != null) {
            viewModel.getCompany(stableDisplaySymbol)
        }
    }

    var lastStat by remember { mutableStateOf<Map<String, Double>>(emptyMap()) }
    val currentStat = createStatSeries(responseData?.companyQuote)

    if (currentStat.isNotEmpty()) {
        lastStat = currentStat
    }

    if (stableDisplaySymbol != null && lastStat.isNotEmpty()) {
        StatsBox(lastStat)
    } else {
        Box(modifier = Modifier.fillMaxSize()) {
            CircularProgressIndicator(modifier = Modifier.align(Alignment.Center))
        }
    }
}

fun createStatSeries(stats: CompanyQuote?): Map<String, Double> {
    if (stats == null) return emptyMap()
    return mapOf(
        "OpenPrice" to stats.OpenPrice,
        "HighPrice" to stats.HighPrice,
        "LowPrice" to stats.LowPrice,
        "PrevClose" to stats.PrevClose
    )
}

@Composable
fun StatsBox(stats: Map<String, Double>) {
    Surface(
        modifier = Modifier
            .fillMaxWidth()
            .padding(vertical = 8.dp)
    ) {
        Text(
            text = "Stats",
            style = MaterialTheme.typography.titleLarge.copy(fontWeight = FontWeight.Bold),
            fontSize = 20.sp,
            modifier = Modifier.padding(horizontal = 4.dp)
        )
        Column(
            modifier = Modifier.padding(16.dp)
        )
        {
            Spacer(modifier = Modifier.height(16.dp))
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween
            ) {
                Column(
                    modifier = Modifier.weight(1f)
                ) {
                    StatItem(label = "Open Price", value = formatPrice(stats["OpenPrice"] ?: 0.0))
                    Spacer(modifier = Modifier.height(8.dp))
                    StatItem(label = "Low Price", value = formatPrice(stats["LowPrice"] ?: 0.0))
                }
                Column(
                    modifier = Modifier.weight(1f)
                ) {
                    StatItem(label = "High Price", value = formatPrice(stats["HighPrice"] ?: 0.0))
                    Spacer(modifier = Modifier.height(8.dp))
                    StatItem(label = "Prev. Close", value = formatPrice(stats["PrevClose"] ?: 0.0))
                }
            }
        }
    }
}

@Composable
fun StatItem(label: String, value: String) {
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.SpaceBetween
    ) {
        Text(
            text = "${label}:",
            style = MaterialTheme.typography.bodyLarge
        )
        Text(
            text = "$$value",
            style = MaterialTheme.typography.bodyLarge,
            modifier = Modifier.padding(end = 10.dp)
        )
    }
}

@Composable
fun AboutBoxInfo(viewModel: CompanyHourlyChartModel, displaySymbol: String?,
                 navController: NavController) {

    val stableDisplaySymbol = remember { displaySymbol }
    val responseData by viewModel.hourlyResults.collectAsState()

    LaunchedEffect(stableDisplaySymbol) {
        if (stableDisplaySymbol != null) {
            viewModel.getCompany(stableDisplaySymbol)
        }
    }

    var lastData by remember { mutableStateOf<Map<String, String>>(emptyMap()) }
    val currentData = createAboutSeries(responseData?.companyData)
    var companyPeers by remember { mutableStateOf<List<String>>(emptyList())}

    if (currentData.isNotEmpty()) {
        lastData = currentData
        companyPeers = responseData?.companyPeer!!
    }

    if (stableDisplaySymbol != null && lastData.isNotEmpty()) {
        AboutBox(lastData, companyPeers, navController)
    }
    else {
        Box(modifier = Modifier.fillMaxSize()) {
            CircularProgressIndicator(modifier = Modifier.align(Alignment.Center))
        }
    }
}

fun createAboutSeries(datas: CompanyData?): Map<String, String> {
    if (datas == null) return emptyMap()
    return mapOf(
        "IPOStartDate" to datas.IPOStartDate,
        "Industry" to datas.Industry,
        "Webpage" to datas.Webpage
    )
}

@Composable
fun AboutBox(aboutData: Map<String, String>, peer:  List<String>?,navController: NavController) {
    Surface(
        modifier = Modifier
            .fillMaxWidth()
            .padding(vertical = 8.dp),
    ) {
        Text(
            text = "About",
            style = MaterialTheme.typography.titleLarge.copy(fontWeight = FontWeight.Bold),
            fontSize = 20.sp,
            modifier = Modifier.padding(horizontal = 4.dp)
        )
    }
        Column(modifier = Modifier.padding(horizontal = 16.dp))
        {
            AboutDetail(label = "IPO Start Date", detail = aboutData["IPOStartDate"] ?: "Not available")
            AboutDetail(label = "Industry", detail = aboutData["Industry"] ?: "Not available")
            AboutDetail(label = "Webpage", detail = aboutData["Webpage"] ?: "Not available", isLink = true)
            AboutDetailPeer(label = "Company Peers", details = peer, navController = navController)
        }

}

@Composable
fun AboutDetailPeer(label: String, details: List<String>?, navController: NavController) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .padding(vertical = 4.dp),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.CenterVertically
    ) {
        Column(modifier = Modifier.weight(1.2f)) {
            Text(
                text = label,
                style = MaterialTheme.typography.bodyLarge,
                color = MaterialTheme.colorScheme.onSurfaceVariant,
                modifier = Modifier.padding(end = 8.dp)
            )
        }
        LazyRow(
            modifier = Modifier.weight(2f),
            horizontalArrangement = Arrangement.spacedBy(8.dp),
            contentPadding = PaddingValues(horizontal = 8.dp)
        ) {
            items(details ?: listOf()) { detail ->
                Text(
                    text = detail,
                    style = TextStyle(
                        color = Color.Blue,
                        fontSize = 16.sp,
                        textDecoration = TextDecoration.Underline
                    ),
                    modifier = Modifier
                        .clickable {
                            navController.navigate("searchResult/${detail}") {
                                popUpTo("mainScreen") { inclusive = true }
                            }
                        }
                )
            }
        }
    }
}


@Composable
fun AboutDetail(label: String, detail: String, isLink: Boolean = false) {
    val context = LocalContext.current

    Row(
        modifier = Modifier
            .fillMaxWidth()
            .padding(vertical = 4.dp),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.CenterVertically
    ) {
        Column {
            Text(
                text = label,
                style = MaterialTheme.typography.bodyLarge,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
        }
        Column {
            if (isLink) {
                Text(
                    text = detail,
                    style = TextStyle(
                        color = Color.Blue,
                        fontSize = 16.sp,
                        textDecoration = TextDecoration.Underline
                    ),
                    modifier = Modifier.clickable {
                        val intent = Intent(Intent.ACTION_VIEW).setData(Uri.parse(detail))
                        context.startActivity(intent)
                    }
                )
            } else {
                Text(
                    text = detail,
                    style = MaterialTheme.typography.bodyMedium,
                    color = MaterialTheme.colorScheme.onSurface
                )
            }
        }
    }
}

@Composable
fun InsightBoxInfo(viewModel: CompanyHourlyChartModel, displaySymbol: String?) {

    val stableDisplaySymbol = remember { displaySymbol }
    val responseData by viewModel.hourlyResults.collectAsState()

    LaunchedEffect(stableDisplaySymbol) {
        if (stableDisplaySymbol != null) {
            viewModel.getCompany(stableDisplaySymbol)
        }
    }

    var lastInsight by remember { mutableStateOf<Map<String, Double>>(emptyMap()) }
    val currentInsight = createInsightSeries(responseData?.companyInsider)

    if (currentInsight.isNotEmpty()) {
        lastInsight = currentInsight
    }

    val compayName = responseData?.companyData?.CompanysName

    if (stableDisplaySymbol != null && lastInsight.isNotEmpty()) {
        InsiderSentimentsTableStyled(lastInsight,compayName)
    }
    else {
        Box(modifier = Modifier.fillMaxSize()) {
            CircularProgressIndicator(modifier = Modifier.align(Alignment.Center))
        }
    }
}

fun createInsightSeries(insights: List<CompanyInsider>?): Map<String, Double> {
    if (insights == null || insights.isEmpty()) return emptyMap()
    val sumChange = insights.fold(0.0) { acc, item -> acc + item.change }          // from ChatGPT
    val sumPositiveChange = insights.fold(0.0) { acc, item ->
        if (item.change > 0) acc + item.change else acc
    }
    val sumNegativeChange = insights.fold(0.0) { acc, item ->
        if (item.change < 0) acc + item.change else acc
    }
    val sumMspr = insights.fold(0.0) { acc, item -> acc + item.mspr }
    val sumPositiveMspr = insights.fold(0.0) { acc, item ->
        if (item.mspr > 0) acc + item.mspr else acc
    }
    val sumNegativeMspr = insights.fold(0.0) { acc, item ->
        if (item.mspr < 0) acc + item.mspr else acc
    }
    return mapOf(
        "Total Change" to sumChange,
        "Positive Change" to sumPositiveChange,
        "Negative Change" to sumNegativeChange,
        "Total MSPR" to sumMspr,
        "Positive MSPR" to sumPositiveMspr,
        "Negative MSPR" to sumNegativeMspr
    )
}

@Composable
fun InsiderSentimentsTableStyled(TableData: Map<String, Double>, Name:  String?) {
    Surface(
        modifier = Modifier
            .fillMaxWidth()
            .padding(vertical = 8.dp)

    )
    {
        Column(modifier = Modifier.padding(vertical = 16.dp)) {
            Text(
                text = "Insights",
                style = MaterialTheme.typography.titleLarge.copy(fontWeight = FontWeight.Bold),
                fontSize = 20.sp,
                modifier = Modifier.padding(horizontal = 4.dp)
            )
            Text(
                text = "Social Sentiments",
                fontSize = 20.sp,
                color = Color(0xFF555454),
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(
                        top = 5.dp,
                        bottom = 5.dp
                    ),
                textAlign = TextAlign.Center
            )
            Box(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(top = 5.dp, bottom = 10.dp)
                    .background(Color.White)
            ) {
                Column(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(bottom = 5.dp)
                        .align(Alignment.Center)
                ) {
                    TableHeader(Name)
                    TableContent(TableData)
                }
            }
        }
    }
}

@Composable
fun TableHeader(Name:  String?) {
    val headerBackgroundColor = Color.LightGray
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .padding(vertical = 1.dp, horizontal = 2.dp),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.CenterVertically
    ) {
        TableCellStyled(Name ?: "Name", Modifier.weight(1f), headerBackgroundColor)
        Spacer(modifier = Modifier.width(2.dp))
        TableCellStyled("MSPR", Modifier.weight(1f), headerBackgroundColor)
        Spacer(modifier = Modifier.width(2.dp))
        TableCellStyled("Change", Modifier.weight(1f), headerBackgroundColor)
    }
}

@Composable
fun TableContent(insightData: Map<String, Double>) {
    val dataRows = listOf(
        "Total" to Pair(insightData["Total MSPR"] ?: 0.0, insightData["Total Change"] ?: 0.0),
        "Positive" to Pair(insightData["Positive MSPR"] ?: 0.0, insightData["Positive Change"] ?: 0.0),
        "Negative" to Pair(insightData["Negative MSPR"] ?: 0.0, insightData["Negative Change"] ?: 0.0)
    )
    Column {
        dataRows.forEach { (label, values) ->
            TableRowStyled(label, values.first, values.second)
        }
    }
}

@Composable
fun TableRowStyled(label: String, mspr: Double, change: Double) {
    val headerBackgroundColor = Color.LightGray
    val headerBackgroundColor2 = Color(0xFFF0F0F0)
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .padding(vertical = 1.dp, horizontal = 2.dp),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.CenterVertically
    ) {
        TableCellStyled(text = label, modifier = Modifier.weight(1f), backgroundColor = headerBackgroundColor)
        Spacer(modifier = Modifier.width(2.dp))
        TableCellStyled(text = formatPrice(mspr), modifier = Modifier.weight(1f), backgroundColor = headerBackgroundColor2)
        Spacer(modifier = Modifier.width(2.dp))
        TableCellStyled(text = formatPrice(change), modifier = Modifier.weight(1f), backgroundColor = headerBackgroundColor2)
    }
}

@Composable
fun TableCellStyled(text: String, modifier: Modifier, backgroundColor: Color) {
    Box(
        modifier = modifier
            .background(backgroundColor)
            .padding(horizontal = 10.dp),
        contentAlignment = Alignment.Center
    ) {
        Text(
            text = text,
            textAlign = TextAlign.Center,
            modifier = Modifier
                .fillMaxWidth()
                .padding(vertical = 6.dp)
        )
    }
}

fun formatPrice(value: Double): String {
    return "%.2f".format(value)
}


@Composable
fun NewsBoxInfo(viewModel: CompanyHourlyChartModel, displaySymbol: String?) {

    val stableDisplaySymbol = remember { displaySymbol }
    val responseData by viewModel.hourlyResults.collectAsState()

    LaunchedEffect(stableDisplaySymbol) {
        stableDisplaySymbol?.let {
            viewModel.getCompany(it)
        }
    }

    var lastNews by remember { mutableStateOf<List<NewsItem>>(emptyList()) }
    val currentNews = responseData?.newsSet
    if (!currentNews.isNullOrEmpty()) {
        lastNews = currentNews
    }

    if (stableDisplaySymbol != null) {
        if (lastNews.isNotEmpty()) {
            NewsBox(lastNews)
        }
        else {
            Box(modifier = Modifier.fillMaxSize()) {
                CircularProgressIndicator(modifier = Modifier.align(Alignment.Center))
            }
        }
    }
    else {
        Box(modifier = Modifier.fillMaxSize()) {
            CircularProgressIndicator(modifier = Modifier.align(Alignment.Center))
        }
    }
}

@Composable
fun NewsBox(newsItems: List<NewsItem>) {
    var showDialog by remember { mutableStateOf(false) }
    var selectedNewsItem by remember { mutableStateOf<NewsItem?>(null) }

    Column(modifier = Modifier.padding(8.dp)) {
        Text(
            text = "News",
            style = MaterialTheme.typography.titleLarge,
            modifier = Modifier.padding(bottom = 16.dp)
        )

        newsItems.filter { it.Image != "" }.take(20).forEachIndexed { index, newsItem ->    // from ChatGPT
            if (index == 0) {
                NewsItemCard(newsItem, onClick = {
                    selectedNewsItem = newsItem
                    showDialog = true
                })
            } else {
                NewsItemCardRegular(newsItem, onClick = {
                    selectedNewsItem = newsItem
                    showDialog = true
                })
            }
        }
    }

    if (showDialog && selectedNewsItem != null) {
        NewsDetailDialog(newsItem = selectedNewsItem!!, onClose = { showDialog = false })
    }
}


@Composable
fun NewsDetailDialog(newsItem: NewsItem, onClose: () -> Unit) {
    val uriHandler = LocalUriHandler.current
    Dialog(onDismissRequest = onClose,
        properties = DialogProperties(
            usePlatformDefaultWidth = false
        )
    ) {
        Card(
            shape = RoundedCornerShape(12.dp),
            modifier = Modifier.fillMaxWidth().padding(horizontal = 6.dp),
            colors = CardDefaults.cardColors(containerColor = Color.White)
        ) {
            Column(modifier = Modifier.fillMaxWidth().padding(16.dp)) {
                Text(
                    text = newsItem.Source,
                    fontWeight = FontWeight.Bold
                )
                Text(text = formatDate(newsItem.PublishedDate))
                Divider(modifier = Modifier.padding(vertical = 8.dp))
                Text(
                    text = newsItem.Title,
                    fontWeight = FontWeight.Bold
                )
                Spacer(modifier = Modifier.height(15.dp))
                Text(text = newsItem.Description)

                Spacer(modifier = Modifier.height(15.dp))
                Row(modifier = Modifier.fillMaxWidth(), verticalAlignment = Alignment.CenterVertically) {
                    Spacer(Modifier.weight(0.5f))
                    Image(
                        painter = painterResource(id = R.drawable.icons8_chrome_48),
                        contentDescription = "Open Link",
                        modifier = Modifier
                            .clickable { uriHandler.openUri(newsItem.LinkToOriginalPost) }
                            .size(48.dp)
                    )
                    Spacer(Modifier.weight(1f))
                    TwitterShareButton("Check out this link - ${newsItem.LinkToOriginalPost}")
                    Spacer(Modifier.weight(1f))
                    FacebookShareButton(newsItem.LinkToOriginalPost)
                    Spacer(Modifier.weight(0.5f))
                }
            }
        }
    }
}

fun shareToTwitter(text: String, context: android.content.Context) {
    val tweetUrl = "https://twitter.com/intent/tweet?text=${Uri.encode(text)}"
    val intent = Intent(Intent.ACTION_VIEW, Uri.parse(tweetUrl))
    context.startActivity(intent)
}

@Composable
fun TwitterShareButton(message: String) {
    val context = LocalContext.current
    val painter = painterResource(id = R.drawable.icons8_twitterx_50)
    IconButton(onClick = { shareToTwitter(message, context) }) {
        Image(
            painter = painter,
            contentDescription = "Share on Twitter",
            modifier = Modifier.size(94.dp)
        )
    }
}

fun shareToFacebook(link: String, context: android.content.Context) {
    val facebookUrl = "https://www.facebook.com/sharer/sharer.php?u=${Uri.encode(link)}"
    val intent = Intent(Intent.ACTION_VIEW, Uri.parse(facebookUrl))
    context.startActivity(intent)
}

@Composable
fun FacebookShareButton(link: String) {
    val context = LocalContext.current
    val painter = painterResource(id = R.drawable.icons8_facebook_96)
    IconButton(onClick = { shareToFacebook(link, context) }) {
        Image(
            painter = painter,
            contentDescription = "Share on Facebook",
            modifier = Modifier.size(84.dp)
        )
    }
}

@Composable
fun NewsItemCard(newsItem: NewsItem, onClick: () -> Unit) {

    Card(
        modifier = Modifier
            .padding(4.dp)
            .clickable(onClick = onClick),
        shape = RoundedCornerShape(8.dp),
        colors = CardDefaults.cardColors(containerColor = Color.White),
        elevation = CardDefaults.cardElevation(defaultElevation = 8.dp)
        )
    {
        Column(modifier = Modifier.padding(8.dp)) {
            Image(
                painter = rememberImagePainter(newsItem.Image),
                contentDescription = "News Image",
                modifier = Modifier
                    .height(150.dp)
                    .fillMaxWidth()
                    .clip(RoundedCornerShape(10.dp)),
                contentScale = ContentScale.Crop
            )
            Spacer(modifier = Modifier.height(8.dp))
            Spacer(modifier = Modifier.height(4.dp))
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween
            ) {
                Text(
                    text = newsItem.Source,
                    style = MaterialTheme.typography.bodySmall,
                    color = Color.Gray,
                    fontSize = 12.sp
                )
                Text(
                    text = formatDate(newsItem.PublishedDate),
                    style = MaterialTheme.typography.bodySmall,
                    color = Color.Gray,
                    fontSize = 12.sp
                )
            }
            Text(
                text = newsItem.Title,
                style = MaterialTheme.typography.titleMedium,
                maxLines = 2,
                overflow = TextOverflow.Ellipsis
            )
        }
    }
}

@Composable
fun NewsItemCardRegular(newsItem: NewsItem, onClick: () -> Unit) {

    Card(
        modifier = Modifier
            .padding(4.dp)
            .fillMaxWidth()
            .heightIn(min = 120.dp)
            .clickable(onClick = onClick),
        shape = RoundedCornerShape(8.dp),
        colors = CardDefaults.cardColors(containerColor = Color.White),
        elevation = CardDefaults.cardElevation(defaultElevation = 8.dp)

    ) {
        Row(modifier = Modifier.padding(8.dp)) {
            Column(
                modifier = Modifier
                    .weight(3f)
                    .padding(end = 8.dp)
                    .heightIn(min = 100.dp),
                verticalArrangement = Arrangement.SpaceAround
            ) {
                Spacer(modifier = Modifier.height(4.dp))
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.SpaceBetween
                ) {
                    Text(
                        text = newsItem.Source,
                        style = MaterialTheme.typography.bodySmall,
                        color = Color.Gray,
                        fontSize = 12.sp
                    )
                    Text(
                        text = formatDate(newsItem.PublishedDate),
                        style = MaterialTheme.typography.bodySmall,
                        color = Color.Gray,
                        fontSize = 12.sp
                    )
                }
                Text(
                    text = newsItem.Title,
                    style = MaterialTheme.typography.titleMedium,
                    maxLines = 2,
                    overflow = TextOverflow.Ellipsis
                )
            }
            Image(
                painter = rememberImagePainter(newsItem.Image),
                contentDescription = "News Image",
                modifier = Modifier
                    .weight(1.2f)
                    .fillMaxHeight()
                    .height(100.dp)
                    .clip(RoundedCornerShape(10.dp)),
                contentScale = ContentScale.Crop
            )
        }
    }
}

fun formatDate(timestamp: Long): String {
    val millis = timestamp * 1000
    val formatter = SimpleDateFormat("MMM dd, yyyy")
    formatter.timeZone = TimeZone.getTimeZone("America/Los_Angeles")
    return formatter.format(Date(millis))
}

@Composable
fun SearchResult(navController: NavController, displaySymbol: String?) {
    val companyWatchModel = viewModel<CompanyWatchModel>()
    val companyHourlyChartModel = viewModel<CompanyHourlyChartModel>()
    val watchListModel = viewModel<WatchlistViewModel>()
    val portfolioViewModel = viewModel<PortfolioInteractModel>()
    val watchModel = viewModel<PortfolioCompanyWatchModel>()
    val interactModel = viewModel<PortfolioInteractModel>()
    var selectedChart by rememberSaveable { mutableStateOf("chart") }

    Column(modifier = Modifier.verticalScroll(rememberScrollState()).fillMaxSize()) {
        TopNav(navController, displaySymbol, watchListModel)
        Spacer(modifier = Modifier.height(16.dp))
        BasicInfo(companyWatchModel, displaySymbol)
        if (selectedChart == "chart") {
            HourlyChartInfo(companyHourlyChartModel, displaySymbol, true, companyWatchModel)
        }
        if (selectedChart == "clock") {
            YearlyChartInfo(companyHourlyChartModel, displaySymbol, true)
        }
        ImageSelectionRow(selectedChart, onImageSelected = { selectedChart = it })
        PortfolioTradeBox(portfolioViewModel, displaySymbol, watchModel, interactModel)
        StatsBoxInfo(companyHourlyChartModel, displaySymbol)
        AboutBoxInfo(companyHourlyChartModel, displaySymbol, navController)
        InsightBoxInfo(companyHourlyChartModel, displaySymbol)
        RecommendationChartInfo(companyHourlyChartModel, displaySymbol)
        EarningChartInfo(companyHourlyChartModel, displaySymbol)
        NewsBoxInfo(companyHourlyChartModel, displaySymbol)
    }
}


@Preview(showBackground = true)
@Composable
fun TryPreview() {
    Ass4Theme {

    }
}






