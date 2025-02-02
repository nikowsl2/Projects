package com.example.ass4.api



import com.example.ass4.api.models.CompanyProfile
import com.example.ass4.api.models.CompanySearchResult
import com.example.ass4.api.models.ResponseData
import com.google.gson.annotations.SerializedName
import retrofit2.Response
import retrofit2.http.Body
import retrofit2.http.GET
import retrofit2.http.HTTP
import retrofit2.http.POST
import retrofit2.http.Query


interface BalanceApiService {
    @GET("api/portfolio/tickers")
    suspend fun getPortfolioList(): Response<List<TickerItem>>

    @GET("api/portfolio/search")
    suspend fun getPortfolioDetails(@Query("ticker") ticker: String): Response<List<PortfolioItem>>

    @POST("api/portfolio/purchase")
    suspend fun purchaseShares(@Body request: PurchaseRequest): Response<PurchaseResponse>

    @POST("api/portfolio/sell")
    suspend fun sellShares(@Body request: SellRequest): Response<SellResponse>

    @GET("api/balance/search")
    suspend fun getBalance(): Response<BalanceResponse>

    @POST("api/balance/update")
    suspend fun updateBalance(@Body updateRequest: BalanceUpdateRequest): Response<String>

    @GET("api/watchlist/getList")
    suspend fun getWatchlist(): Response<List<Ticker>>

    @GET("/search-company")
    suspend fun searchCompany(@Query("q") query: String): CompanySearchResult

    @GET("/watch-company")
    suspend fun watchCompany(@Query("company_id") companyId: String): CompanyProfile

    @GET("/get-company")
    suspend fun getCompany(@Query("company_id") companyId: String): ResponseData

    @POST("/api/watchlist/insert")
    suspend fun insertTicker(@Body request: TickerRequest): Response<TickerResponse>

    @HTTP(method = "DELETE", path = "/api/watchlist/delete", hasBody = false)
    suspend fun removeTicker(@Query("ticker") ticker: String): Response<Unit>
}

data class BalanceUpdateRequest(
    val balance: Double
)

data class PurchaseRequest(
    val companyTicker: String,
    val companyName: String,
    val numberOfShares: Int,
    val sharePrice: Double
)

data class PurchaseResponse(
    val message: String
)

data class SellRequest(
    val ticker: String,
    val sellPrice: Double,
    val numberOfSharesToSell: Int
)

data class SellResponse(
    val message: String,
    val totalSaleValue: Double
)

data class TickerRequest(
    val ticker: String
)

data class TickerResponse(
    val message: String,
    val _id: String?
)

data class ObjectId(
    @SerializedName("\$oid")
    val oid: String
)

data class PortfolioItem(
    val id: ObjectId,
    val ticker: String,
    val name: String,
    val shares: Int,
    val purchasePrice: Double,
    val cumulativeCost: Double
)

data class TickerItem(
    @SerializedName("_id")
    val ticker: String
)

data class BalanceResponse(
    val balance: Double
)

data class Ticker(
    val ticker: String
)

data class AggregatedPortfolioItem(
    val ticker: String,
    val totalShares: Int,
    val averageCost: Double,
    val totalCost: Double
)